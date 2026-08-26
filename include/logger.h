#include <iostream>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <string>
#include <chrono>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
};

class Logger {
public:
    static Logger& getInstance();
    void setLogLevel(LogLevel level);
    void setOutputFile(const std::string& filename);
    void log(LogLevel level, const std::string& message);

private:
    Logger() = default;
    ~Logger();

    std::string levelToString(LogLevel level);
    std::mutex mtx;
    std::ofstream logFile;
    LogLevel currentLogLevel = LogLevel::INFO;
    std::string logFileDir = "../logs";  // Default log directory
};

#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg)  Logger::getInstance().log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::ERROR, msg)
#define LOG_FATAL(msg) Logger::getInstance().log(LogLevel::FATAL, msg)