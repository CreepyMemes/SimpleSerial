#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>
#include <vector>
#include <queue>

#include "Message.h"
#include "helpers.h"
#include "Logging.h"

// SimpleSerial protocol constants
#define SIMPLE_SERIAL_TIMEOUT    50   // Defines the timeout range for each interaction
#define SIMPLE_SERIAL_STACK_SIZE 2048 // Defines the allocated stack size of the main task
#define SIMPLE_SERIAL_CORE       1    // Defines which core the simple serial task will be pinned in
#define SIMPLE_SERIAL_QUEUE_SIZE 10   // Defines the maximum amount of messages held in it's queue

// Define the handshake states
enum Handshake {
    SYN,     // Synchronize
    SYN_ACK, // Synchronize-Acknowledge
    ACK,     // Acknowledge
    NAK,     // Negative Acknowledge
    FIN      // Finish
};

// TODO, TIMEOUT ERROR LOGIC
// start_time = millis()
// (millis() - start_time >= SIMPLE_SERIAL_TIMEOUT);

// Simple Serial protocol API class
class SimpleSerial {
    public:
        SimpleSerial(HardwareSerial *serial, const int8_t rx_pin, const int8_t tx_pin, uint8_t max_retries = 3, const UBaseType_t task_priority = 5);
        ~SimpleSerial();

        void begin(const unsigned long baud_rate, const SerialConfig mode = SERIAL_8N1);
        void end();

        void send(const std::vector<uint8_t> &message);

    private:
        HardwareSerial *_serial; // Pointer to the UART protocol interface instance

        int8_t _rx_pin; // Defined pin for reading UART data
        int8_t _tx_pin; // Defined pin for writing UART data

        uint8_t _max_retries;       // The amount of retries if the protocol fails
        UBaseType_t _task_priority; // Holds the main task's task_priority for the FreeRTOS scheduler
        TaskHandle_t _task_handle;  // Task handle of the outgoing commands task

        std::queue<std::vector<uint8_t> *> _message_queue; // Queue that handles outgoing messages
        QueueHandle_t _freertos_message_queue;             // Queue's handle of the outgoing messages freertos queue

        bool _isAvailableToSend(Message &cmd);
        bool _isAvailableToReceive();

        void _pushQueue(const std::vector<uint8_t> &message);
        void _popQueue(std::vector<uint8_t> *message);

        void _createMessageQueue();
        void _destroyMessageQueue();

        void _createMainTask();
        void _destroyMainTask();

        static void _mainTask(void *arg);
};

#endif // SIMPLE_SERIAL_H