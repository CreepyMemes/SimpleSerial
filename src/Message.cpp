#include "Message.h"

// -------------------------------------- PUBLIC METHODS -----------------------------------------------------

// Default constructor with member initialization set to 0
Message::Message() :
    _mode(MessageMode::UNDEFINED),
    _message(),
    _checksum(0),
    _size(0),
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

    // Verify if received checksum is correct, throw error if not
    _verifyChecksum();
}

// Sender's method that creates a payload with calculated checksum
void Message::create(const std::vector<uint8_t> &message) {
    _mode     = MessageMode::SENDER;
    _message  = message;
    _checksum = _calculateChecksum();
    _initPayloadArray();
}


// Checksum getter method
uint8_t Message::getChecksum() const {
    return _checksum;
}

// Receiver's method that returns a string containing extracted message bytes in hex values
std::vector<uint8_t> Message::getMessage() const {
    if (_mode != MessageMode::RECEIVER) {
        throw std::runtime_error("Attempted to get message while not in RECEIVER mode!");
    }
    return _message;
}

// Sender's method that returns a pointer to the dynamically allocated payload array
uint8_t *Message::getPayload() const {
    if (_mode != MessageMode::SENDER) {
        throw std::runtime_error("Attempted to get payload array while not in SENDER mode!");
    }
    return _payload;
}

// Sender's method that return the size of the allocated payload array
size_t Message::getSize() const {
    if (_mode != MessageMode::SENDER) {
        throw std::runtime_error("Attempted to get size of payload array while not in SENDER mode!");
    }
    return _size;
}

// -------------------------------------- PRIVATE METHODS -----------------------------------------------------

// Calculates the CRC-8/ROHC checksum for the given data
uint8_t Message::_calculateChecksum() const {
    return ~esp_crc8_le(0, const_cast<const uint8_t *>(_message.data()), _message.size());
}

// Calculates the checksum based on received message, throws error if received checksum and calculated don't match
void Message::_verifyChecksum() const {
    if (_mode != MessageMode::RECEIVER) {
        throw std::runtime_error("Attempted to verify checksum while not in RECEIVER mode!");
    }

    // Calculate new checksum based on received message
    uint8_t checksum = _calculateChecksum();

    // Throw error if checksums don't match
    if (_checksum != checksum) {
        char errorMsg[100];
        std::snprintf(errorMsg, sizeof(errorMsg), "Checksum verification failed, received: 0x%02X, expected: 0x%02X", _checksum, checksum);
        throw std::runtime_error(errorMsg);
    }
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
        _size    = 0;
    }
}