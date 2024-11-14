#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>
#include <esp_crc.h>
#include <vector>
#include "Logging.h"

// Define the message structure
class Message {
    public:
        Message();

        void decode(const std::vector<uint8_t> &payload);
        void create(const std::vector<uint8_t> &message);

        bool verify() const;
        std::vector<uint8_t> genPayload() const;

        // Accessor method
        std::vector<uint8_t> getMessage() const;
        uint8_t getChecksum() const;

    private:
        std::vector<uint8_t> _message;
        uint8_t _checksum; // Checksum of the message data

        uint8_t _calculateChecksum() const;
};

#endif // MESSAGE_H