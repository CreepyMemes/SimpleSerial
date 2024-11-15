#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>
#include <esp_crc.h>
#include <vector>
#include <string>
#include <cstring> // for memcpy

#include "Logging.h"

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

        bool verify() const;

        // Accessor method
        uint8_t getChecksum() const;
        std::string getMessage() const;
        uint8_t *getPayload() const;
        size_t getSize() const;

    private:
        MessageMode _mode;             // Saves which mode the object is currently in
        std::vector<uint8_t> _message; // Message data
        uint8_t _checksum;             // Checksum of the message data
        uint8_t *_payload;             // The payload array with message and checksum
        size_t _size;                  // The payload size

        uint8_t _calculateChecksum() const;

        void _initPayloadArray();
        void _destroyPayloadArray();

        std::string _ToHexString(uint8_t byte) const;
};

#endif // MESSAGE_H