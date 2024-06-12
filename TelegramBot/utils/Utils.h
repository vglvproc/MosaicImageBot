#pragma once
#include <string>
#include <vector>

std::string getRandomHexValue_16();

std::string getRandomHexValue(uint16_t length);

int64_t getRandomInt64(int64_t min, int64_t max);

int32_t getRandomInt32(int32_t min, int32_t max);

int16_t getRandomInt16(int16_t min, int16_t max);

std::vector<std::string> getRandomHexList(uint32_t count, uint16_t length, bool unique);