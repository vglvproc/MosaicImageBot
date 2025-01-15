#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <iomanip>

enum class LogSource {
    Main,
    Database,
    Bot,
    Other
};

std::string logSourceToString(LogSource source);

class Logger {
public:
    static void log(LogSource source, std::ostream &stream, const std::string &message);
    static void logToFile(LogSource source, const std::string &filename, const std::string &message);

private:
    static std::mutex mutex_;
};

#endif