#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>
#include <SimpleTimeOut.h>

// Define binary commands using an enum
enum Command : uint8_t {
    CMD_START = 0x01,
    CMD_STOP = 0x02,
    CMD_STATUS = 0x03,
    CMD_RESET = 0x04,
};

class SimpleSerial {
    public:
        SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, const int8_t cts_pin, const uint8_t rts_pin,
                     const uint32_t stack_size = 4096, const UBaseType_t priority = 5, const uint64_t timeout_duration = 1000, uint8_t max_retries = 3);
        ~SimpleSerial();

        void begin(const unsigned long baud_rate, const SerialConfig mode = SERIAL_7E2);
        void sendCommand(const Command cmd);

    private:
        HardwareSerial *_serial; // Pointer to the UART protocol interface instance

        int8_t _pin_tx;
        int8_t _pin_rx;
        int8_t _pin_cts;
        int8_t _pin_rts;

        uint32_t _stack_size_task;  // Defines the allocated stack size of the main task
        UBaseType_t _priority_task; // Holds the main task's priority for the FreeRTOS scheduler

        QueueHandle_t _queue_cmds_out; // Queue's handle of the outgoing commands queue
        TaskHandle_t _handle_task;     // Task handle of the outgoing commands task

        SimpleTimeOut _timeout;
        uint8_t _max_retries;

        bool _is_cmd_to_send(const Command &cmd);
        bool _is_cmd_to_receive();

        bool _request_to_send();
        bool _is_cmd_sent(const Command cmd);
        bool _is_send_mode_done();
        bool _is_sender_success(const Command cmd);
        bool _sender_retry(const Command cmd);

        static void _task_main(void *pvParameters);

        static constexpr const char *TAG = "simple_serial";
};

#endif // SIMPLE_SERIAL_H