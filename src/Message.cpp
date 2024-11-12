#include "Message.h"

Message::Message() :
    _length(0),
    _checksum(0) {
}

// Receiver's constructor to set the message data received checksum
bool Message::set(const uint8_t *data, const size_t length, uint8_t checksum) {

    // Data too large to fit in the buffer
    if (length > MESSAGE_MAX_DATA_SIZE) {
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

uint8_t Message::_calculateChecksum() const {
    return ~esp_crc8_le(0, _data, _length);
}
