#include "logger.h"

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(LogLevel level) {
    currentLogLevel = level;
}

void Logger::setOutputFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx);
    if (logFile.is_open()) {
        logFile.close();
    }
    logFile.open(logFileDir + "/" + filename, std::ios::out | std::ios::app);
    if (!logFile) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < currentLogLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(mtx);
    if (!logFile.is_open()) {
        std::cerr << "Log file is not open. Please set an output file." << std::endl;
        return;
    }
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::string outputMessage = "[" + std::to_string(time) + "] [" + levelToString(level) + "] " + message + "\n";
    
    if (logFile.is_open()) {
        logFile << outputMessage;
        logFile.flush();
    } else {
        std::cerr << "Log file is not open. Please set an output file." << std::endl;
    }
    std::cout << outputMessage;
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}
