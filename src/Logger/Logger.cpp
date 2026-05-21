#include <Logger/Logger.hpp>

#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

Logger::Logger(
        LogLevel minLogLevel,
        LogMode logMode,
        std::string_view filename) {
    minLogLevel_ = minLogLevel;
    stream_ = create_stream(logMode, filename);

    if (!stream_ || !stream_->good()) {
        throw std::runtime_error("Logger: no se pudo inicializar el stream");
    }
}

Logger& Logger::operator<<(std::ostream& (*manip)(std::ostream&)) {
    *stream_ << manip;
    return *this;
}

std::ostream& Logger::stream() noexcept {
    return *stream_;
}

void Logger::log(LogLevel level, std::string_view message) {
    if (level >= minLogLevel_) {
        std::string prefix;
        prefix.reserve(32);
        prefix += '[';
        prefix += logLevelToString(level);
        prefix += "] [";
        prefix += getCurrentTime();
        prefix += "] ";

        *stream_ << prefix << message << std::endl;
    }
}

std::unique_ptr<std::ostream> Logger::create_stream(
        LogMode mode,
        std::string_view filename) {
    switch (mode)
    {
        case LogMode::CONSOLE:
            return std::make_unique<std::ostream>(std::cerr.rdbuf());

        case LogMode::FILE: {
            auto fs = std::make_unique<std::ofstream>(
              std::string(filename),
              std::ios::out | std::ios::app
            );

            if (!fs->is_open()) {
              throw std::runtime_error(
                  "Logger: no se pudo abrir el fichero: " + std::string(filename)
              );
            }

            return fs;
        }
    }
    return nullptr;
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "     ";
    }
}

std::string Logger::getCurrentTime() {
    time_t now = time(nullptr);
    tm* tm_local = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp),
             "%d-%m-%Y %H:%M:%S", tm_local);

    return std::string(timestamp);
}
