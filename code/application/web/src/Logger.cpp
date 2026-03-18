#include "Logger.h"

Logger::Logger(const std::string& filename, bool consoleOutput)
    : consoleOutput(consoleOutput) 
{
    logFile.open(filename, std::ios::app);
    if (!logFile) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case ERROR:   return "ERROR";
        case WARNING: return "WARNING";
        case INFO:    return "INFO";
        case DEBUG:   return "DEBUG";
        default:      return "UNKNOWN";
    }
}

Logger& Logger::getInstance(const std::string& filename, bool consoleOutput) {
    static Logger instance(filename, consoleOutput);
    return instance;
}
