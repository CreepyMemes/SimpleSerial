#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>
#include <esp_crc.h>

// Message class constants
#define MESSAGE_MAX_DATA_SIZE 128

// Define the message structure
class Message {
    public:
        Message();

        bool set(const uint8_t *data, const size_t len, uint8_t checksum);
        bool set(const uint8_t *data, const size_t len);

        bool verify() const;

    private:
        size_t _length;     // Length of the message data
        uint8_t _data[128]; // Message data byte buffer with fixed size
        uint8_t _checksum;  // Checksum of the message data

        uint8_t _calculateChecksum() const; // Calculate the CRC-8/ROHC checksum for the given data
};

#endif // MESSAGE_H