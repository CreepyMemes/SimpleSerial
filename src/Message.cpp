#include "Message.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

// Default constructor with member initialization set to 0
Message::Message() :
    _mode(MessageMode::UNDEFINED),
    _message(),
    _checksum(0),
    _payload(NULL) {
}

// Destroyer method, deallocates payload array
Message::~Message() {
    _destroyPayloadArray();
}

// Receiver's method to decodes a received payload extracting message and checksum
void Message::decode(const std::vector<uint8_t> &payload) {
    _mode    = MessageMode::RECEIVER;
    _message = payload;
    _message.pop_back();
    _checksum = payload.back();
    _destroyPayloadArray();
}

// Sender's method that creates a payload with calculated checksum
void Message::create(const std::vector<uint8_t> &message) {
    _mode     = MessageMode::SENDER;
    _message  = message;
    _checksum = _calculateChecksum();
    _initPayloadArray();
}

// Receiver's method to check if the checksum of the decoded message is correct
bool Message::verify() const {
    if (_mode == MessageMode::RECEIVER) {
        uint8_t checksum = _calculateChecksum();

        if (checksum == _checksum) {
            return true;
        }
        // SS_LOG_E("Checksum verification failed, received: 0x%02x, expected: 0x%02x", _checksum, checksum);
        return false;
    }

    SS_LOG_W("Attempted to verify checksum in SENDER MODE");
    return 0;
}

// Checksum getter method
uint8_t Message::getChecksum() const {
    return _checksum;
}

// Receiver's method that returns a string containing extracted message bytes in hex values
std::string Message::getMessage() const {
    if (_mode == MessageMode::RECEIVER) {
        std::string str = "";
        for (uint8_t i : _message) str += _ToHexString(i);
        str.pop_back();
        return str;
    }

    SS_LOG_W("Attempted to get message string in SENDER MODE");
    return "";
}

// Sender's method that returns a pointer to the dynamically allocated payload array
uint8_t *Message::getPayload() const {
    if (_mode == MessageMode::SENDER) {
        return _payload;
    }

    SS_LOG_W("Attempted to get payload array in RECEIVER MODE");
    return NULL;
}

// Sender's method that return the size of the allocated payload array
size_t Message::getSize() const {
    if (_mode == MessageMode::SENDER) {
        return _size;
    }

    SS_LOG_W("Attempted to get payload array size in RECEIVER MODE");
    return 0;
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------

// Calculates the CRC-8/ROHC checksum for the given data
uint8_t Message::_calculateChecksum() const {
    return ~esp_crc8_le(0, const_cast<const uint8_t *>(_message.data()), _message.size());
}

// Dynamically allocates the payload array ready to be sent
void Message::_initPayloadArray() {
    _destroyPayloadArray();
    _size    = _message.size() + 1;
    _payload = new uint8_t[_size];
    std::memcpy(_payload, _message.data(), _message.size() * sizeof(uint8_t));
    _payload[_size - 1] = _checksum;
}

// Destroys the previously allocated payload array if any
void Message::_destroyPayloadArray() {
    if (_payload != NULL) {
        delete[] _payload;
        _payload = NULL;
    }
}

// Helper function that converts a byte into a hex string
std::string Message::_ToHexString(uint8_t byte) const {
    char hexStr[5];
    sprintf(hexStr, "0x%02x ", byte);
    return std::string(hexStr);
}