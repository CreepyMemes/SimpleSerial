#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>

// SimpleSerial protocol constants
#define SIMPLE_SERIAL_TIMEOUT 50

// Defined TAG for Serial logging
#define SIMPLE_SERIAL_LOG_TAG "simple_serial"

// Serial logging levels
#define SIMPLE_SERIAL_LOG_LEVEL_NONE 0
#define SIMPLE_SERIAL_LOG_LEVEL_ERROR 1
#define SIMPLE_SERIAL_LOG_LEVEL_WARN 2
#define SIMPLE_SERIAL_LOG_LEVEL_INFO 3
#define SIMPLE_SERIAL_LOG_LEVEL_DEBUG 4

// Serial logging toggles
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

// Defined binary commands using an enum
enum Command : uint8_t {
    CMD_START = 0x01,
    CMD_STOP = 0x02,
    CMD_STATUS = 0x03,
    CMD_RESET = 0x04,
    CMD_TEST = 0x69,
    CMD_WRONG = 0x45
};

// TODO: add a way for user to know if command was sent successfully, or to know when a command is available
// TODO: fix
class SimpleSerial {
    public:
        SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, const int8_t cts_pin, const uint8_t rts_pin,
                     const uint32_t task_stack_size = 4096, const UBaseType_t task_priority = 5, uint8_t max_retries = 3);
        ~SimpleSerial();

        void begin(const unsigned long baud_rate, const SerialConfig mode = SERIAL_8N1);
        void send(const Command cmd);

    private:
        HardwareSerial *_serial; // Pointer to the UART protocol interface instance

        // SimpleSerial protocol defined pins
        int8_t _rx_pin;
        int8_t _tx_pin;
        int8_t _cts_pin;
        int8_t _rts_pin;

        uint32_t _task_stack_size;  // Defines the allocated stack size of the main task
        UBaseType_t _task_priority; // Holds the main task's task_priority for the FreeRTOS scheduler

        QueueHandle_t _queue_commands_to_send; // Queue's handle of the outgoing commands queue
        TaskHandle_t _task_handle;             // Task handle of the outgoing commands task

        uint8_t _max_retries; // The amount of retries if the protocol fails
        uint64_t _start_time; // Time in ms used for setting timeout

        // Timeout methods
        void startTimeout();
        bool isTimeout(const uint64_t reduce_factor = 1);

        // Exit protocol modes methods
        void exitMode(const char *mode);
        bool isPeerExit(const char *peer_role);

        // Handshaking protocol
        void sendConfirmation();
        bool isConfirmed();
        bool isConfirmedACK();
        void endConfirmation();

        // Sender Mode methods
        bool isNewCommandToSend(Command &cmd);
        bool requestToSend();
        void sendCommand(const Command cmd);
        bool isEchoCorrect(const Command cmd);
        bool isSenderSuccess(const Command cmd);
        bool senderRetry(const Command cmd);

        // Receiver Mode methods
        bool isNewCommandToReceive();
        void acceptRequest();
        bool isCommandReceived(Command &cmd);
        void sendCommandEcho(const Command cmd);
        bool isReceivalSuccess(Command &cmd);
        bool receiverRetry(Command &cmd);

        // Main protocol looping async task
        static void _task_main(void *pvParameters);
};

#endif // SIMPLE_SERIAL_H