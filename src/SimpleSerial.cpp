#include "SimpleSerial.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

SimpleSerial::SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, const int8_t cts_pin, const uint8_t rts_pin,
                           const uint32_t task_stack_size, const UBaseType_t task_priority, uint8_t max_retries) :
    _serial(serial),
    _rx_pin(rx_pin),
    _tx_pin(tx_pin),
    _cts_pin(cts_pin),
    _rts_pin(rts_pin),
    _task_stack_size(task_stack_size),
    _task_priority(task_priority),
    _max_retries(max_retries),
    _start_time(0) {

    _task_handle = NULL;                                         // Initialize the main task's handle to null
    _queue_commands_to_send = xQueueCreate(10, sizeof(Command)); // Create the queue that holds outgoing messages
}

SimpleSerial::~SimpleSerial() {
    vTaskDelete(_task_handle);
    vQueueDelete(_queue_commands_to_send);
}

void SimpleSerial::begin(const unsigned long baud_rate, const SerialConfig mode) {
    _serial->begin(baud_rate, mode, _rx_pin, _tx_pin); // Initialize the Hardware Serial Instance on the given port by argument

    pinMode(_cts_pin, INPUT); // Initialize the "Clear To Send" pin to INPUT LOW -> DEFAULT

    pinMode(_rts_pin, OUTPUT);   // Initialize the "Request To Send" pin
    digitalWrite(_rts_pin, LOW); // and set it to LOW -> DEFAULT

    xTaskCreatePinnedToCore(_task_main, "simple_serial", _task_stack_size, this, _task_priority, &_task_handle, 1); // Create a FreeRTOS task for asynchronous operation
}

void SimpleSerial::send(const Command cmd) {
    xQueueSend(_queue_commands_to_send, &cmd, (TickType_t)10);
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------

void SimpleSerial::startTimeout() {
    _start_time = millis();
}

bool SimpleSerial::isTimeout(const uint64_t reduce_factor) {
    return (millis() - _start_time >= SIMPLE_SERIAL_TIMEOUT / reduce_factor);
}

void SimpleSerial::exitMode(const char *mode) {
    digitalWrite(_rts_pin, LOW);
    LOG_D("Exited from %s Mode", mode);
}

bool SimpleSerial::isPeerExit(const char *peer_role) {
    LOG_D("Awaiting %s to exit...", peer_role);

    startTimeout();
    while (!isTimeout()) {
        if (digitalRead(_cts_pin) == LOW) {
            LOG_D("%s exited!", peer_role);
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    LOG_W("Timeout, %s didn't exit!", peer_role);
    return false;
}

void SimpleSerial::sendConfirmation() {
    digitalWrite(_rts_pin, LOW);
    LOG_D("Sent confirmation!");
}

bool SimpleSerial::isConfirmed() {
    LOG_D("Awaiting for confirmation...");

    startTimeout();
    while (!isTimeout()) {
        if (digitalRead(_cts_pin) == LOW) {
            LOG_D("Success! Confirmation Received!");
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    LOG_W("Timeout! did not receive any confirmation!");
    return false;
}

bool SimpleSerial::isConfirmedACK() {
    LOG_D("Awaiting for confirmation acknowledgement...");

    startTimeout();
    while (!isTimeout()) {
        if (digitalRead(_cts_pin) == HIGH) {
            LOG_D("Success!, Confirmation acknowledgement received!");
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    LOG_W("Timeout! did not receive any confirmation acknowledgement!");
    return false;
}

void SimpleSerial::endConfirmation() {
    LOG_D("Confirmation protocol ended");
    digitalWrite(_rts_pin, HIGH);
}

// -------------------------------------- SENDER METHODS -----------------------------------------------------

bool SimpleSerial::isNewCommandToSend(Command &cmd) {
    if (xQueueReceive(_queue_commands_to_send, (void *)&cmd, (TickType_t)10) == pdTRUE) {
        LOG_I("New command to be sent: 0x%x", cmd);
        return true;
    }

    return false;
}

bool SimpleSerial::requestToSend() {
    digitalWrite(_rts_pin, HIGH);
    LOG_D("Entered Sender Mode, awaiting permission to send...");

    startTimeout();
    while (!isTimeout()) {

        if (digitalRead(_cts_pin) == HIGH) {
            LOG_D("Successful request! received permission to send!");
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    LOG_W("Failed request! Timeout, did not receive permission to send!");
    return false;
}

void SimpleSerial::sendCommand(const Command cmd) {
    _serial->write((uint8_t *)&cmd, sizeof(cmd));
    LOG_D("Attempted to send command: 0x%x", cmd);
}

bool SimpleSerial::isEchoCorrect(const Command cmd) {

    LOG_D("Awaiting receiver to respond by echoing same command...");
    Command response;

    startTimeout();
    while (!isTimeout(2)) {
        if (_serial->available()) {
            response = (Command)_serial->read();

            if (cmd == response) {
                LOG_D("Command sent successfully! Received correct response: 0x%x", response);
                return true;
            } else {
                LOG_W("Command failed to send! received response 0x%x instead of 0x%x", response, cmd);
                return false;
            }
        }

        delay(1); // To avoid flooding the CPU
    }

    LOG_W("Command failed to send! Timeout, did not receive any response!");
    return false;
}

bool SimpleSerial::isSenderSuccess(const Command cmd) {

    // Check if the other ESP32 accepted the request to send a command
    if (requestToSend()) {

        // Attempt to send the command
        sendCommand(cmd);

        // Check if the command was sent successfully
        if (isEchoCorrect(cmd)) {

            // Send confirmation by just setting RTS pin to LOW
            sendConfirmation();

            // Check if the sent confirmation has been received
            if (isConfirmed()) {

                // End confirmation protocol by just setting RTS pin back to HIGH
                endConfirmation();

                // Wait until receiver exits (for sync)
                if (isPeerExit("Receiver")) {

                    // Exit after receiver exits
                    exitMode("Sender");

                    LOG_I("Sender protocol completed successfully!\n");
                    return true;
                }
            }
        }
    }

    // End confirmation as it went wrong
    endConfirmation();

    // Now wait until the receiver exits for sync
    isPeerExit("Receiver");

    // Exit after receiver exits
    exitMode("Sender");

    LOG_E("Sender protocol failed!\n");
    return false;
}

bool SimpleSerial::senderRetry(const Command cmd) {

    // Retry to send the message if it fails
    for (int attempt = 0; attempt < _max_retries; attempt++) {

        if (isSenderSuccess(cmd)) {
            return true;
        }

        if (attempt != _max_retries - 1) {
            LOG_I("Attempt: %d, Retrying to send...", attempt + 2);
        }

        delay(1); // To avoid flooding the CPU
    }

    return false;
}

// -------------------------------------- RECEIVER METHODS -----------------------------------------------------

bool SimpleSerial::isNewCommandToReceive() {
    if (digitalRead(_cts_pin) == HIGH) {
        LOG_I("Received a new sender request!");
        return true;
    }

    return false;
}

void SimpleSerial::acceptRequest() {
    digitalWrite(_rts_pin, HIGH);
    LOG_D("Request accepted!");
}

bool SimpleSerial::isCommandReceived(Command &cmd) {
    LOG_D("Ready to receive, waiting for command...");

    startTimeout();
    while (!isTimeout()) {
        if (_serial->available() > 0) {
            cmd = (Command)_serial->read();
            LOG_D("Received command: 0x%x", cmd);
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    LOG_W("Timeout, didn't receive any command!");
    return false;
}

void SimpleSerial::sendCommandEcho(const Command cmd) {
    _serial->write((uint8_t *)&cmd, sizeof(cmd));
    LOG_D("Echoed back same command: 0x%x", cmd);
}

bool SimpleSerial::isReceivalSuccess(Command &cmd) {

    // Accept the sender request by the other ESP32, by setting RTS pin HIGH
    acceptRequest();

    // Check if a command has been received
    if (isCommandReceived(cmd)) {

        // Echo back command for confirmation
        sendCommandEcho(cmd);

        // Check if the sender confirms the echoed command being correct
        if (isConfirmed()) {

            // Send confirmation that sender's confirmation was received
            sendConfirmation();

            // Check if the sender received our confirmation acknowledgement
            if (isConfirmedACK()) {

                // End confirmation protocol by just setting RTS pin back to HIGH
                endConfirmation();

                // Exit receiver mode
                exitMode("Receiver");

                // Wait until sender exits as well (for sync)
                if (isPeerExit("Sender")) {
                    LOG_I("Receiver protocol completed successfully!\n");
                    return true;
                }
            }
        }
    }

    // Confirmation failed just end it
    endConfirmation();

    // Exit receiver mode
    exitMode("Receiver");

    // Wait until sender exits as well (for sync)
    isPeerExit("Sender");

    LOG_E("Receiver protocol failed!\n");
    return false;
}

bool SimpleSerial::receiverRetry(Command &cmd) {

    // Take count of the receiving attempts if they fail
    for (int attempt = 0; attempt < _max_retries; attempt++) {

        if (isReceivalSuccess(cmd)) {
            return true;
        }

        if (attempt != _max_retries - 1) {
            LOG_I("Attempt: %d, awaiting sender to retry...", attempt + 2);
        }

        delay(1); // To avoid flooding the CPU
    }

    return false;
}

// -------------------------------------- MAIN TASK -----------------------------------------------------

void SimpleSerial::_task_main(void *pvParameters) {
    SimpleSerial *self = (SimpleSerial *)pvParameters; // Cast pvParameters to SimpleSerial pointer
    Command cmd;

    while (true) {

        // Check if there's a command to be sent by this ESP32
        if (self->isNewCommandToSend(cmd)) {

            // If sending fails, retry for max_retries times
            if (self->senderRetry(cmd)) {
                // Command sent successfully logic here
                Serial.print("Successfully sent command: 0x");
                Serial.print(cmd, HEX);
                Serial.println('\n');
            }
        }
        // Check if there's a request to receive a command sent by the other ESP32
        else if (self->isNewCommandToReceive()) {

            // If receival fails, take count of the attempts (max_retries must be same)
            if (self->receiverRetry(cmd)) {

                // Command received successfully logic here
                Serial.print("Executing command: 0x");
                Serial.print(cmd, HEX);
                Serial.println('\n');
            }
        }

        delay(1); // Give other tasks a chance to run
    }
}
