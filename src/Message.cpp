#include "Message.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

// Default constructor with member initialization set to 0
Message::Message() :
    _message(),
    _checksum(0) {
}

// Receiver's method to decodes a received payload extracting checksum
void Message::decode(const std::vector<uint8_t> &payload) {
    _message = payload;
    _message.pop_back();
    _checksum = payload.back();
}

// Sender's method to creates a payload with calculated checksum
void Message::create(const std::vector<uint8_t> &message) {
    _message  = message;
    _checksum = _calculateChecksum();
}

// Receiver's method to check if the checksum of the decoded message is correct
bool Message::verify() const {
    return _calculateChecksum() == _checksum;
}

// Sender's method to generate a message payload
std::vector<uint8_t> Message::genPayload() const {
    std::vector<uint8_t> payload = _message;
    payload.push_back(_checksum);
    return payload;
}

// Message getter method
std::vector<uint8_t> Message::getMessage() const {
    return _message;
}

// Checksum getter method
uint8_t Message::getChecksum() const {
    return _checksum;
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------

// Calculate the CRC-8/ROHC checksum for the given data
uint8_t Message::_calculateChecksum() const {
    return ~esp_crc8_le(0, const_cast<const uint8_t *>(&_message[0]), _message.size());
}