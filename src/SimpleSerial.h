#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>

// clang-format off
#define SIMPLE_SERIAL_TIMEOUT 50
#define SIMPLE_SERIAL_LOG_TAG "simple_serial"

#define SIMPLE_SERIAL_LOG_LEVEL_NONE  0
#define SIMPLE_SERIAL_LOG_LEVEL_ERROR 1
#define SIMPLE_SERIAL_LOG_LEVEL_WARN  2
#define SIMPLE_SERIAL_LOG_LEVEL_INFO  3
#define SIMPLE_SERIAL_LOG_LEVEL_DEBUG 4

#ifndef SIMPLE_SERIAL_LOG_LEVEL
    #define SIMPLE_SERIAL_LOG_LEVEL SIMPLE_SERIAL_LOG_LEVEL_NONE
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_ERROR
    #define LOG_E(format, ...) ESP_LOGE(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
    #define LOG_E(format, ...)
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_WARN
    #define LOG_W(format, ...) ESP_LOGW(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
    #define LOG_W(format, ...)
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_INFO
    #define LOG_I(format, ...) ESP_LOGI(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
    #define LOG_I(format, ...)
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_DEBUG
    #define LOG_D(format, ...) ESP_LOGD(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
    #define LOG_D(format, ...)
#endif
// clang-format on

// Define binary commands using an enum
enum Command : uint8_t {
    CMD_START = 0x01,
    CMD_STOP = 0x02,
    CMD_STATUS = 0x03,
    CMD_RESET = 0x04,
    CMD_TEST = 0x69,
    CMD_WRONG = 0x45
};

// TODO: add a way for user to know if command was sent successfully, or to know when a command is available
class SimpleSerial {
    public:
        SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, const int8_t cts_pin, const uint8_t rts_pin,
                     const uint32_t stack_size = 4096, const UBaseType_t priority = 5, uint8_t max_retries = 3);
        ~SimpleSerial();

        void begin(const unsigned long baud_rate, const SerialConfig mode = SERIAL_7E1);
        void send(const Command cmd);

    private:
        HardwareSerial *_serial; // Pointer to the UART protocol interface instance

        int8_t _pin_rx;
        int8_t _pin_tx;
        int8_t _pin_cts;
        int8_t _pin_rts;

        uint32_t _stack_size_task;  // Defines the allocated stack size of the main task
        UBaseType_t _priority_task; // Holds the main task's priority for the FreeRTOS scheduler

        QueueHandle_t _queue_cmds_out; // Queue's handle of the outgoing commands queue
        TaskHandle_t _handle_task;     // Task handle of the outgoing commands task

        uint8_t _max_retries;
        uint64_t _start_time;

        void _start_timeout();
        bool _is_timeout(const uint64_t reduce_factor = 1);

        void _exit_mode(const char *mode);
        bool _is_peer_exit(const char *peer_role);
        void _send_confirmation();
        bool _is_confirmed();
        bool _is_confirmed_ack();
        void _end_confirmation();

        bool _is_cmd_to_send(const Command &cmd);
        bool _request_to_send();
        void _send_command(const Command cmd);
        bool _is_echo_correct(const Command cmd);
        bool _is_sender_success(const Command cmd);
        bool _sender_retry(const Command cmd);

        bool _is_cmd_to_receive();
        void _accept_request();
        bool _is_cmd_received(Command &cmd);
        void _send_cmd_echo(Command cmd);
        bool _is_receival_success(Command &cmd);
        bool _receiver_retry(Command &cmd);

        static void _task_main(void *pvParameters);
};

#endif // SIMPLE_SERIAL_H