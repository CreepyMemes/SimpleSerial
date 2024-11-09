#include "SimpleSerial.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

SimpleSerial::SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, const int8_t cts_pin, const uint8_t rts_pin,
                           const uint32_t stack_size, const UBaseType_t priority, uint8_t max_retries) :
    _serial(serial),
    _pin_rx(rx_pin),
    _pin_tx(tx_pin),
    _pin_cts(cts_pin),
    _pin_rts(rts_pin),
    _stack_size_task(stack_size),
    _priority_task(priority),
    _timeout(SIMPLE_SERIAL_TIMEOUT),
    _max_retries(max_retries) {

    _handle_task = NULL;                                 // Initialize the main task's handle to null
    _queue_cmds_out = xQueueCreate(10, sizeof(Command)); // Create the queue that holds outgoing messages
}

SimpleSerial::~SimpleSerial() {
    vTaskDelete(_handle_task);
    vQueueDelete(_queue_cmds_out);
}

void SimpleSerial::begin(const unsigned long baud_rate, const SerialConfig mode) {
    _serial->begin(baud_rate, mode, _pin_rx, _pin_tx); // Initialize the Hardware Serial Instance on the given port by argument

    pinMode(_pin_cts, INPUT_PULLDOWN); // Initialize the "Clear To Send" pin to INPUT LOW -> DEFAULT

    pinMode(_pin_rts, OUTPUT);   // Initialize the "Request To Send" pin
    digitalWrite(_pin_rts, LOW); // and set it to LOW -> DEFAULT

    xTaskCreatePinnedToCore(_task_main, "simple_serial", _stack_size_task, this, _priority_task, &_handle_task, 1); // Create a FreeRTOS task for asynchronous operation
}

void SimpleSerial::send(const Command cmd) {
    xQueueSend(_queue_cmds_out, &cmd, (TickType_t)10);
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------

void SimpleSerial::_exit_mode(const char *mode) {
    digitalWrite(_pin_rts, LOW);
    ESP_LOGD(TAG, "Exited from %s Mode", mode);
}

bool SimpleSerial::_is_peer_exit(const char *peer_role) {
    ESP_LOGD(TAG, "Awaiting %s to exit...", peer_role);

    _timeout.start();
    while (!_timeout.isExpired()) {
        if (digitalRead(_pin_cts) == LOW) {
            ESP_LOGD(TAG, "%s exited!", peer_role);
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    ESP_LOGW(TAG, "Timeout, %s didn't exit!", peer_role);
    return false;
}

void SimpleSerial::_send_confirmation() {
    digitalWrite(_pin_rts, LOW);
    ESP_LOGD(TAG, "Sent confirmation pulse!");
}

bool SimpleSerial::_is_confirmed() {
    ESP_LOGD(TAG, "Awaiting for confirmation...");

    _timeout.start();
    while (!_timeout.isExpired()) {
        if (digitalRead(_pin_cts) == LOW) {
            ESP_LOGD(TAG, "Confirmation Received!");
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    ESP_LOGW(TAG, "Timeout, didn't receive any confirmation reply!");
    return false;
}

bool SimpleSerial::_is_confirmed_reply() {
    ESP_LOGD(TAG, "Awaiting for sender to confirm receiving our confirmation of their confirmation...");

    _timeout.start();
    while (!_timeout.isExpired()) {
        if (digitalRead(_pin_cts) == HIGH) {
            ESP_LOGD(TAG, "Confirmation received!");
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    ESP_LOGW(TAG, "Timeout, didn't receive any confirmation reply!");
    return false;
}

void SimpleSerial::_end_confirmation() {
    digitalWrite(_pin_rts, HIGH);
}

// -------------------------------------- SENDER METHODS -----------------------------------------------------

bool SimpleSerial::_is_cmd_to_send(const Command &cmd) {
    if (xQueueReceive(_queue_cmds_out, (void *)&cmd, (TickType_t)10) == pdTRUE) {
        ESP_LOGI(TAG, "New command to be sent: 0x%x", cmd);
        return true;
    }

    return false;
}

bool SimpleSerial::_request_to_send() {
    digitalWrite(_pin_rts, HIGH);
    ESP_LOGD(TAG, "Entered Sender Mode, awaiting permission to send...");

    _timeout.start();
    while (!_timeout.isExpired()) {

        if (digitalRead(_pin_cts) == HIGH) {
            ESP_LOGD(TAG, "Permission to send granted!");
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    ESP_LOGW(TAG, "Timeout, request failed!");
    return false;
}

void SimpleSerial::_send_command(const Command cmd) {
    _serial->write((uint8_t *)&cmd, sizeof(cmd));
    ESP_LOGD(TAG, "Attempted to send command");
}

bool SimpleSerial::_is_echo_correct(const Command cmd) {

    ESP_LOGD(TAG, "Awaiting command receival confirmation...");
    Command response;

    _timeout.start();
    while (!_timeout.isExpired()) {
        if (_serial->available()) {
            response = (Command)_serial->read();

            if (cmd == response) {
                ESP_LOGD(TAG, "Sent command has been confirmed!");
                return true;
            } else {
                ESP_LOGD(TAG, "Failed to confirm sent command, received: 0x%x", response);
                return false;
            }
        }

        delay(1); // To avoid flooding the CPU
    }

    ESP_LOGW(TAG, "Timeout, receiver did not send any confirmation!");
    return false;
}

bool SimpleSerial::_is_sender_success(const Command cmd) {

    // Check if the other ESP32 accepted the request to send a command
    if (_request_to_send()) {

        // Attempt to send the command
        _send_command(cmd);

        // Check if the command was sent successfully
        if (_is_echo_correct(cmd)) {

            // Send confirmation by just setting RTS pin to LOW
            _send_confirmation();

            // Check if the sent confirmation has been received
            if (_is_confirmed()) {

                // End confirmation protocol by just setting RTS pin back to HIGH
                _end_confirmation();

                // Check if the receiver exited
                if (_is_peer_exit("Receiver")) {

                    // Exit after receiver exits
                    _exit_mode("Sender");
                    return true;
                }
            }
        }
    }

    // End confirmation as it went wrong
    _end_confirmation();

    // Now wait until the receiver exits for sync
    _is_peer_exit("Receiver");

    // Exit after receiver exits
    _exit_mode("Sender");

    ESP_LOGE(TAG, "Sender protocol failed!\n");
    return false;
}

bool SimpleSerial::_sender_retry(const Command cmd) {

    // Retry to send the message if it fails
    for (int attempt = 0; attempt < _max_retries; attempt++) {

        if (_is_sender_success(cmd)) {
            ESP_LOGI(TAG, "Sender protocol completed successfully after %d attempts!\n", attempt + 1);
            return true;
        }

        if (attempt != _max_retries - 1) {
            ESP_LOGW(TAG, "Retrying to send...\n");
        }

        delay(1); // To avoid flooding the CPU
    }

    return false;
}

// -------------------------------------- RECEIVER METHODS -----------------------------------------------------

bool SimpleSerial::_is_cmd_to_receive() {
    if (digitalRead(_pin_cts) == HIGH) {
        ESP_LOGI(TAG, "Received a new sender request!");
        return true;
    }

    return false;
}

void SimpleSerial::_accept_request() {
    digitalWrite(_pin_rts, HIGH);
    ESP_LOGD(TAG, "Request accepted!");
}

bool SimpleSerial::_is_cmd_received(Command &cmd) {
    ESP_LOGD(TAG, "Ready to receive, waiting for command...");

    _timeout.start();
    while (!_timeout.isExpired()) {
        if (_serial->available() > 0) {
            cmd = (Command)_serial->read();
            ESP_LOGD(TAG, "Received command: 0x%x", cmd);
            return true;
        }

        delay(1); // To avoid flooding the CPU
    }

    ESP_LOGW(TAG, "Timeout, didn't receive any command!");
    return false;
}

void SimpleSerial::_send_cmd_echo(const Command cmd) {
    _serial->write((uint8_t *)&cmd, sizeof(cmd));
    ESP_LOGD(TAG, "Echoed back command: 0x%x", cmd);
}

// Will not retry if it fails, that's the sender's job
bool SimpleSerial::_is_receival_success() {
    Command cmd;

    // Accept the sender request by the other ESP32, by setting RTS pin HIGH
    _accept_request();

    // Check if a command has been received
    if (_is_cmd_received(cmd)) {

        // cmd = CMD_WRONG; // intentional bug

        // Echo back command for confirmation
        _send_cmd_echo(cmd);

        // Check if the sender confirms the echoed command being correct
        if (_is_confirmed()) {

            // Send confirmation that sender's confirmation was received
            _send_confirmation();

            // Check if the sender received our confirmation of receiving their confirmation o.O?
            if (_is_confirmed_reply()) {

                // End confirmation protocol by just setting RTS pin back to HIGH
                _end_confirmation();

                // Exit receiver mode
                _exit_mode("Receiver");

                // Wait until sender exits as well (for sync)
                if (_is_peer_exit("Sender")) {
                    ESP_LOGI(TAG, "Receiver protocol completed successfully!\n");
                    return true;
                }
            }
        }
    }

    // Confirmation failed just end it
    _end_confirmation();

    // Exit receiver mode
    _exit_mode("Receiver");

    // Wait until sender exits as well (for sync)
    _is_peer_exit("Sender");

    ESP_LOGE(TAG, "Receiver protocol failed!\n");
    return false;
}

// -------------------------------------- MAIN TASK -----------------------------------------------------

void SimpleSerial::_task_main(void *pvParameters) {
    SimpleSerial *self = (SimpleSerial *)pvParameters; // Cast pvParameters to SimpleSerial pointer
    Command cmd;

    while (true) {

        // Check if there's a command to be sent by this ESP32
        if (self->_is_cmd_to_send(cmd)) {

            // If sending fails, halt program execution (FOR NOW, TO REMOVE LATER)
            if (!self->_sender_retry(cmd)) {
                ESP_LOGE(TAG, "Sender protocol failed after %d attempts!\n", self->_max_retries);
            }
        }
        // Check if there's a request to receive a command sent by the other ESP32
        else if (self->_is_cmd_to_receive()) {

            // If receival fails, just print it for debugging
            if (self->_is_receival_success()) {
                // message received successfully do something here
            }
        }

        delay(1); // Give other tasks a chance to run
    }
}
