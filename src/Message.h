#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>
#include <esp_crc.h>
#include <vector>

// Message class for managing sending and receiving binary messages with checksum
class Message {
    public:
        Message();

        void decode(const std::vector<uint8_t> &payload);
        void create(const std::vector<uint8_t> &message);

        bool verify() const;
        std::vector<uint8_t> genPayload() const;

        // Accessor methods
        std::vector<uint8_t> getMessage() const;
        uint8_t getChecksum() const;

    private:
        std::vector<uint8_t> _message; // Vector that holds the message
        uint8_t _checksum;             // Checksum of the message data

        uint8_t _calculateChecksum() const;
};

#endif // MESSAGE_H