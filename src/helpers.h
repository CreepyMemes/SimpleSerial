#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>
#include <vector>

std::string intToHexString(uint8_t block);
std::string vectorToHexString(std::vector<uint8_t> *vec);

#endif