#include "helpers.h"

// Helper function that converts an uint8_t block to a hex string
std::string intToHexString(uint8_t block) {
    char str[5];
    sprintf(str, "0x%02x ", block);
    return std::string(str);
}

// Helper function that converts a pointer of a vector of uint8_t elements to a string of the values in hex delimited by a space
std::string vectorToHexString(std::vector<uint8_t> *vec) {
    std::string str = "";
    for (uint8_t i : *vec) str += intToHexString(i);
    str.pop_back();
    return str;
}