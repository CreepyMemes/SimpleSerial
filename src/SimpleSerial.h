#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>
#include "Message.h"

// SimpleSerial protocol constants
#define SIMPLE_SERIAL_TIMEOUT    50   // Defines the timeout range for each interaction
#define SIMPLE_SERIAL_STACK_SIZE 2048 // Defines the allocated stack size of the main task
#define SIMPLE_SERIAL_CORE       1    // Defines which core the simple serial task will be pinned in

// Defined TAG for Serial logging
#define SIMPLE_SERIAL_LOG_TAG "simple_serial"

// Serial logging levels
#define SIMPLE_SERIAL_LOG_LEVEL_NONE  0
#define SIMPLE_SERIAL_LOG_LEVEL_ERROR 1
#define SIMPLE_SERIAL_LOG_LEVEL_WARN  2
#define SIMPLE_SERIAL_LOG_LEVEL_INFO  3
#define SIMPLE_SERIAL_LOG_LEVEL_DEBUG 4

// Serial logging toggles
#ifndef SIMPLE_SERIAL_LOG_LEVEL
#define SIMPLE_SERIAL_LOG_LEVEL SIMPLE_SERIAL_LOG_LEVEL_NONE
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_ERROR
#define SS_LOG_E(format, ...) ESP_LOGE(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
#define SS_LOG_E(format, ...)
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_WARN
#define SS_LOG_W(format, ...) ESP_LOGW(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
#define SS_LOG_W(format, ...)
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_INFO
#define SS_LOG_I(format, ...) ESP_LOGI(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
#define SS_LOG_I(format, ...)
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_DEBUG
#define SS_LOG_D(format, ...) ESP_LOGD(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
#define SS_LOG_D(format, ...)
#endif

// Define the handshake states
enum Handshake {
    SYN     = 0x01, // Synchronize
    SYN_ACK = 0x02, // Synchronize-Acknowledge
    ACK     = 0x03, // Acknowledge
    NAK     = 0x04, // Negative Acknowledge
    FIN     = 0x05  // Finish
};

// Simple Serial api class
class SimpleSerial {
    public:
        SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, uint8_t max_retries = 3, const UBaseType_t task_priority = 5);
        ~SimpleSerial();

        void begin(const unsigned long baud_rate, const SerialConfig mode);
        void end();

        void send(const Message &msg);

    private:
        HardwareSerial *_serial; // Pointer to the UART protocol interface instance

        int8_t _rx_pin; // Defined pin for reading UART data
        int8_t _tx_pin; // Defined pin for writing UART data

        TaskHandle_t _task_handle;    // Task handle of the outgoing commands task
        UBaseType_t _task_priority;   // Holds the main task's task_priority for the FreeRTOS scheduler
        QueueHandle_t _message_queue; // Queue's handle of the outgoing commands queue
        uint8_t _max_retries;         // The amount of retries if the protocol fails

        void _createMessageQueue();
        void _destroyMessageQueue();

        void _createMainTask();
        void _destroyMainTask();

        static void _mainTask(void *arg);
};

#endif // SIMPLE_SERIAL_H