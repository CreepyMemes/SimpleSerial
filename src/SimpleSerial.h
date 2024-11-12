#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>
#include "Logging.h"
#include "Message.h"

// SimpleSerial protocol constants
#define SIMPLE_SERIAL_TIMEOUT    50   // Defines the timeout range for each interaction
#define SIMPLE_SERIAL_STACK_SIZE 2048 // Defines the allocated stack size of the main task
#define SIMPLE_SERIAL_CORE       1    // Defines which core the simple serial task will be pinned in

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

        uint8_t _readMessage(uint8_t *msg);
        bool _isAvailableToSend(Message &cmd);
        bool _isAvailableToReceive();

        void _createMessageQueue();
        void _destroyMessageQueue();

        void _createMainTask();
        void _destroyMainTask();

        static void _mainTask(void *arg);
};

#endif // SIMPLE_SERIAL_H