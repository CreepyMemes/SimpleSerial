#include "SimpleSerial.h"

const char *SimpleSerial::TAG = "simple_serial";

SimpleSerial::SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, const int8_t busy_pin, const uint32_t stack_size, const UBaseType_t priority) {
    _serial = serial;
    _pin_tx = tx_pin;
    _pin_rx = rx_pin;
    _pin_busy_line = busy_pin;

    // Create a queue to hold outgoing messages
    _queue_cmds_out = xQueueCreate(10, sizeof(Command));

    // Create a FreeRTOS task for asynchronous operation
    xTaskCreatePinnedToCore(_task_cmds_out, "serial_cmds_out", stack_size, this, priority, &_handle_cmds_out, 1);
}

SimpleSerial::~SimpleSerial() {
    vTaskDelete(_handle_cmds_out);
    vQueueDelete(_queue_cmds_out);
}

void SimpleSerial::begin(const unsigned long baud_rate, const SerialConfig mode) {
    _serial->begin(baud_rate, mode, _pin_rx, _pin_tx);
    pinMode(_pin_busy_line, INPUT);
}

void SimpleSerial::sendCommand(const Command cmd) {
    xQueueSend(_queue_cmds_out, &cmd, (TickType_t)10);
}

// Line busy if HIGH
bool SimpleSerial::_is_line_busy() {
    return digitalRead(_pin_busy_line);
}

void SimpleSerial::_take_line() {
    pinMode(_pin_busy_line, OUTPUT);
    digitalWrite(_pin_busy_line, HIGH);
}

void SimpleSerial::_free_line() {
    digitalWrite(_pin_busy_line, LOW);
    pinMode(_pin_busy_line, INPUT);
}

void SimpleSerial::_write_cmd(const Command &cmd) {
    _take_line();
    _serial->write((uint8_t *)&cmd, sizeof(cmd));
    _free_line();
}

void SimpleSerial::_task_cmds_out(void *pvParameters) {
    Command cmd;
    SimpleSerial *self = static_cast<SimpleSerial *>(pvParameters); // Cast pvParameters to SimpleSerial pointer

    while (true) {
        // Check if line is not busy
        if (!self->_is_line_busy()) {

            // Check if there is data in the outgoing commands queue to write
            if (xQueueReceive(self->_queue_cmds_out, (void *)&cmd, (TickType_t)10) == pdTRUE) {
                self->_write_cmd(cmd);

                ESP_LOGD(TAG, "Sent command: %c", cmd);
            }
        } else {
            ESP_LOGD(TAG, "Line busy!");
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); // Give other tasks a chance to run
    }
}