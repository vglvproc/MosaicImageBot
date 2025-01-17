#include "Logger.h"

std::mutex Logger::mutex_;

std::string logSourceToString(LogSource source) {
    static const std::unordered_map<LogSource, std::string> sourceMap = {
        {LogSource::Main, "Main"},
        {LogSource::Database, "Database"},
        {LogSource::Bot, "Bot"},
        {LogSource::Other, "Other"}
    };

    auto it = sourceMap.find(source);
    return (it != sourceMap.end()) ? it->second : "Unknown";
}

void Logger::log(LogSource source, std::ostream &stream, const std::string &message) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &time_t_now);
#else
    localtime_r(&time_t_now, &localTime);
#endif

    const int sourceWidth = 12;

    stream << std::left << std::put_time(&localTime, "%d.%m.%Y %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count()
           << " " << std::setw(sourceWidth) << std::setfill(' ')
           << logSourceToString(source) << message << std::endl;
}

void Logger::logToFile(LogSource source, const std::string &filename, const std::string &message) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ofstream fileStream(filename, std::ios::app);
    if (fileStream.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm localTime;
#ifdef _WIN32
        localtime_s(&localTime, &time_t_now);
#else
        localtime_r(&time_t_now, &localTime);
#endif

        const int sourceWidth = 12;

        fileStream << std::left << std::put_time(&localTime, "%d.%m.%Y %H:%M:%S")
                   << '.' << std::setfill('0') << std::setw(3) << ms.count()
                   << " " << std::setw(sourceWidth) << std::setfill(' ')
                   << logSourceToString(source) << message << std::endl;
    } else {
        std::cerr << "[Logger] Failed to open log file: " << filename << std::endl;
    }
}
