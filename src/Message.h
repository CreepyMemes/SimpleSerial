#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>
#include <esp_crc.h>
#include <vector>
#include <cstring> // for memcpy
#include <stdexcept>

enum MessageMode {
    UNDEFINED,
    SENDER,
    RECEIVER
};

// Define the message structure
class Message {
    public:
        Message();
        ~Message();

        void decode(const std::vector<uint8_t> &payload);
        void create(const std::vector<uint8_t> &message);

        // Accessor method
        uint8_t getChecksum() const;
        std::vector<uint8_t> getMessage() const;
        size_t getSize() const;
        uint8_t *getPayload() const;

    private:
        MessageMode _mode; // Saves which mode the object is currently in

        std::vector<uint8_t> _message; // Message data
        uint8_t _checksum;             // Checksum of the message data

        size_t _size;      // The allocated size of the payload array
        uint8_t *_payload; // The payload array that combines message and checksum

        uint8_t _calculateChecksum() const;
        void _verifyChecksum() const;

        void _initPayloadArray();
        void _destroyPayloadArray();
};

#endif // MESSAGE_H