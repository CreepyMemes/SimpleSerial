#include "SimpleSerial.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

SimpleSerial::SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, const uint32_t task_stack_size, const UBaseType_t task_priority, uint8_t max_retries) :
    _serial(serial),
    _rx_pin(rx_pin),
    _tx_pin(tx_pin),
    _task_stack_size(task_stack_size),
    _task_priority(task_priority),
    _max_retries(max_retries),
    _start_time(0) {

    _task_handle            = NULL;                              // Initialize the main task's handle to null
    _queue_commands_to_send = xQueueCreate(10, sizeof(Command)); // Create the queue that holds outgoing messages
}

SimpleSerial::~SimpleSerial() {
    vTaskDelete(_task_handle);
    vQueueDelete(_queue_commands_to_send);
}

void SimpleSerial::begin(const unsigned long baud_rate, const SerialConfig mode) {
    // Initialize the Hardware Serial Instance on the given port by argument
    _serial->begin(baud_rate, mode, _rx_pin, _tx_pin);

    // Create a FreeRTOS task for asynchronous operation
    xTaskCreatePinnedToCore(_task_main, "simple_serial", _task_stack_size, this, _task_priority, &_task_handle, 1);
}

void SimpleSerial::send(const Command cmd) {
    xQueueSend(_queue_commands_to_send, &cmd, (TickType_t)10);
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------


// -------------------------------------- SENDER METHODS -----------------------------------------------------


// -------------------------------------- RECEIVER METHODS -----------------------------------------------------

// -------------------------------------- MAIN TASK -----------------------------------------------------

void SimpleSerial::_task_main(void *pvParameters) {
    SimpleSerial *self = (SimpleSerial *)pvParameters; // Cast pvParameters to SimpleSerial pointer
    Command cmd;

    while (true) {

        delay(1); // Give other tasks a chance to run
    }
}
