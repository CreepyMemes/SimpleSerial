#include "Message.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

// Message constructor that initializes everything to 0
Message::Message() :
    _length(0),
    _checksum(0) {
    memset(_data, 0, MESSAGE_MAX_DATA_SIZE);
}

// Receiver's constructor to set the message data received checksum
bool Message::set(const uint8_t *data, const size_t length, uint8_t checksum) {

    // Data too large to fit in the buffer
    if (length > MESSAGE_MAX_DATA_SIZE) {
        SS_LOG_E("Error setting message data, size too long, max is %d, got %d", MESSAGE_MAX_DATA_SIZE, length);
        return false;
    }

    _length = length;
    memcpy((void *)_data, data, length);
    _checksum = checksum;

    return true;
}

// Sender's constructor to set the message data with new checksum generation
bool Message::set(const uint8_t *data, const size_t length) {
    return set(data, length, _calculateChecksum());
}

// Method to verify the checksum of the message
bool Message::verify() const {
    return _calculateChecksum() == _checksum;
}


// -------------------------------------- PRIVATE METHODS -----------------------------------------------------

// data buffer getter method
void Message::getData(uint8_t *data) const {
    memcpy(data, _data, _length);
}

// data length getter method
size_t Message::getLength() const {
    return _length;
}

// checksum getter method
uint8_t Message::getChecksum() const {
    return _checksum;
}

// Calculate the CRC-8/ROHC checksum for the given data
uint8_t Message::_calculateChecksum() const {
    return ~esp_crc8_le(0, _data, _length);
}
