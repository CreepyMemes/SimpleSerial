#include "SimpleSerial.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

// Constructor
SimpleSerial::SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, uint8_t max_retries, const UBaseType_t task_priority) :
    _serial(serial),
    _rx_pin(rx_pin),
    _tx_pin(tx_pin),
    _task_priority(task_priority),
    _max_retries(max_retries),
    _task_handle(NULL),
    _message_queue(NULL) {

    // _message_queue = xQueueCreate(10, sizeof(int)); // Create the queue that holds outgoing messages
}

// Destructor
SimpleSerial::~SimpleSerial() {
    end();
}

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

void SimpleSerial::send(const Message &msg) {
    xQueueSend(_message_queue, &msg, (TickType_t)10);
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------

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

// bool isTimeout(unsigned long start_time) {
//     return (millis() - start_time >= SIMPLE_SERIAL_TIMEOUT);
// }


void SimpleSerial::_mainTask(void *arg) {

    // Cast arg to SimpleSerial pointer to go into this object instance
    SimpleSerial *self = (SimpleSerial *)arg;

    while (true) {

        delay(1); // Give other tasks a chance to run
    }
}
