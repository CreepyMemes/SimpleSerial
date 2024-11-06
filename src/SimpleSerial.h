#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>

// Define binary commands using an enum
enum Command : uint8_t {
    CMD_START = 0x01,
    CMD_STOP = 0x02,
    CMD_STATUS = 0x03,
    CMD_RESET = 0x04,
};

class SimpleSerial {
    public:
        SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, const int8_t busy_pin, const uint32_t stack_size = 4096, const UBaseType_t priority = 5);
        ~SimpleSerial();

        static const char *TAG; // Defined TAG for debugging log

        void begin(const unsigned long baud_rate, const SerialConfig mode = SERIAL_7E2);
        void sendCommand(const Command cmd);

    private:
        HardwareSerial *_serial;       // Pointer to the UART protocol interface instance
        QueueHandle_t _queue_cmds_out; // Queue's handle of the outgoing commands queue
        TaskHandle_t _handle_cmds_out; // Task handle of the outgoing commands task

        int8_t _pin_tx;
        int8_t _pin_rx;
        int8_t _pin_busy_line;

        bool _is_line_busy();
        void _take_line();
        void _free_line();
        void _write_cmd(const Command &cmd);

        static void _task_cmds_out(void *pvParameters);
};

#endif // SIMPLE_SERIAL_H