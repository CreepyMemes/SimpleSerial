#include "SimpleSerial.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

// SimpleSerial Constructor
SimpleSerial::SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, uint8_t max_retries, const UBaseType_t task_priority) :
    _serial(serial),
    _rx_pin(rx_pin),
    _tx_pin(tx_pin),
    _task_priority(task_priority),
    _max_retries(max_retries),
    _task_handle(NULL),
    _message_queue(NULL) {
}

// Destructor
SimpleSerial::~SimpleSerial() {
    end();
}

// Initiates the uart protocol, and initiates the async task
void SimpleSerial::begin(const unsigned long baud_rate, const SerialConfig mode) {
    _serial->begin(baud_rate, mode, _rx_pin, _tx_pin);
    _createMessageQueue();
    _createMainTask();
}

// Destroy main task and messages queue
void SimpleSerial::end() {
    _destroyMainTask();
    _destroyMessageQueue();
}

// Send a message through this protocol, it adds it to messages to send queue
void SimpleSerial::send(const Message &msg) {
    xQueueSend(_message_queue, &msg, (TickType_t)10);
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------

// TODO ADD HANDSHAKE AND TEST THIS SHIT ALREADY!!!

uint8_t _readBlock() {
    // todo this is for handshake
}

// TODO this is ugly, make a struct for payload or someshit

// This goes after successful handshake
uint8_t SimpleSerial::_readMessage(uint8_t *msg) {

    uint8_t block_idx = 0; // Variable that will hold the received block indexes
    byte msg_block;        // Variable that stores each received 8 bit block per time

    while (_serial->available() > 0) {
        msg_block        = _serial->read(); // Read byte block
        msg[block_idx++] = msg_block;       // Append byte block

        if (block_idx > MESSAGE_MAX_DATA_SIZE / 8) {
            SS_LOG_I("Error, received message exceeds maximum length");
            return 0;
        }
    }

    return block_idx * sizeof(uint8_t); // Return the received message size
}

bool SimpleSerial::_isAvailableToSend(Message &msg) {
    if (xQueueReceive(_message_queue, &msg, (TickType_t)10) == pdTRUE) {
        SS_LOG_I("New message to be sent with size: %d", msg.getLength());
        return true;
    }

    return false;
}


// TODO
bool SimpleSerial::_isAvailableToReceive() {
    if (_serial->available() > 0) {

        return true;
    }

    return false;
}

// Create the queue that will hold the outoging messages
void SimpleSerial::_createMessageQueue() {
    if (_message_queue == NULL) {
        _message_queue = xQueueCreate(10, sizeof(Message));

        if (_message_queue == NULL) {
            SS_LOG_E("Queue for outgoing messages failed to create!");
        }
    }
}

// Destroy the outgoing messages queue
void SimpleSerial::_destroyMessageQueue() {
    if (_message_queue != NULL) {
        vQueueDelete(_message_queue);
        _message_queue = NULL;
    }
}

// Create the main task for asynchronous operation
void SimpleSerial::_createMainTask() {
    if (_task_handle == NULL) {
        xTaskCreatePinnedToCore(_mainTask, "simple_serial", SIMPLE_SERIAL_STACK_SIZE, this, _task_priority, &_task_handle, SIMPLE_SERIAL_CORE);

        if (_task_handle == NULL) {
            SS_LOG_E("Main task failed to create!");
        }
    }
}

// Destroy the main task
void SimpleSerial::_destroyMainTask() {
    if (_task_handle != NULL) {
        vTaskDelete(_task_handle);
        _task_handle = NULL;
    }
}

// TODO, TIMEOUT ERROR LOGIC
// start_time = millis()
// (millis() - start_time >= SIMPLE_SERIAL_TIMEOUT);

// TODO
void SimpleSerial::_mainTask(void *arg) {

    // Cast arg to SimpleSerial pointer to go into this object instance
    SimpleSerial *self = (SimpleSerial *)arg;

    // Create new message object to hold incoming/outcoming messages
    Message msg;

    while (true) {
        if (self->_isAvailableToSend(msg)) {
            // send msg protocol here
        }

        if (self->_isAvailableToReceive()) {
            // receive msg protocol here
        }

        delay(1); // Give other tasks a chance to run
    }
}
