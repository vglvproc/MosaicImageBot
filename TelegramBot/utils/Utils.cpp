#include "Utils.h"
#include <random>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <unordered_set>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

std::string getRandomHexValue_16() {
    return getRandomHexValue(16);
}

std::string getRandomHexValue(uint16_t length) {
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, 255);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (uint16_t i = 0; i < length / 2 + (length % 2); ++i) {
        ss << std::setw(2) << distr(eng);
    }

    return ss.str().substr(0, length);
}

int64_t getRandomInt64(int64_t min, int64_t max) {
    std::random_device rd;
    std::mt19937_64 eng(rd());
    std::uniform_int_distribution<int64_t> distr(min, max);
    return distr(eng);
}

int32_t getRandomInt32(int32_t min, int32_t max) {
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int32_t> distr(min, max);
    return distr(eng);
}

int16_t getRandomInt16(int16_t min, int16_t max) {
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int16_t> distr(min, max);
    return distr(eng);
}

std::vector<std::string> getRandomHexList(uint32_t count, uint16_t length, bool unique) {
    std::vector<std::string> hexList;
    std::unordered_set<std::string> hexSet;

    if (unique) {
        while (hexList.size() < count) {
            std::string hex = getRandomHexValue(length);
            if (hexSet.find(hex) == hexSet.end()) {
                hexList.push_back(hex);
                hexSet.insert(hex);
            }
        }
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            hexList.push_back(getRandomHexValue(length));
        }
    }

    return hexList;
}

long long getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
}

std::string getFormatTimestampWithMilliseconds(long long timestamp) {
    using namespace std::chrono;

    milliseconds ms{timestamp};
    system_clock::time_point tp{ms};

    time_t time = system_clock::to_time_t(tp);
    auto millis = duration_cast<milliseconds>(tp.time_since_epoch()).count() % 1000;

    std::tm* tm = std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(tm, "%Y.%m.%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << millis;

    return ss.str();
}

std::string getFormatTimestampWithoutMilliseconds(long long timestamp) {
    using namespace std::chrono;

    milliseconds ms{timestamp};
    system_clock::time_point tp{ms};

    time_t time = system_clock::to_time_t(tp);

    std::tm* tm = std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(tm, "%Y.%m.%d %H:%M:%S");

    return ss.str();
}

// TODO: Need to add checking wget accessability at the start of program launching
bool downloadFile(const std::string& url, const std::string& filePath) {
    std::string command = "wget -O " + filePath + " " + url;
    int result = system(command.c_str());
    return result == 0;
}

std::string getFileExtensionFromUrl(const std::string& url) {
    size_t lastSlashPos = url.find_last_of('/');
    size_t lastDotPos = url.find_last_of('.');

    if (lastDotPos != std::string::npos && (lastSlashPos == std::string::npos || lastDotPos > lastSlashPos)) {
        return url.substr(lastDotPos + 1);
    }

    return "";
}

bool createDirectory(const std::string& path) {
    std::string command = "mkdir -p " + path;
    int result = system(command.c_str());
    return result == 0;
}

std::string getCurrentWorkingDir() {
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    } else {
        std::cerr << "Error getting current directory" << std::endl;
        return "";
    }
}

bool doesPathExist(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}