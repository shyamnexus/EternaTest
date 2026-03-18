#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>
#include <ctime>
#include <iomanip>

class Logger {
public:
    enum Level {
        ERROR,
        WARNING,
        INFO,
        DEBUG
    };

    // Get the singleton instance of the Logger
    static Logger& getInstance(const std::string& filename = "logfile.log", bool consoleOutput = true);

    // Delete the copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template <typename... Args>
    void log(Level level, const std::string& message, Args&&... args);

private:
    Logger(const std::string& filename, bool consoleOutput); // Private constructor
    ~Logger();

    std::string levelToString(Level level) const;

    std::ofstream logFile;
    bool consoleOutput;
    std::mutex logMutex;
};

// Template method implementation should be in header
template <typename... Args>
void Logger::log(Level level, const std::string& message, Args&&... args) {
    std::lock_guard<std::mutex> lock(logMutex);

    // Get the current time
    std::time_t now = std::time(nullptr);
    char timeBuf[20];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    // Use stringstream for formatting
    std::ostringstream oss;
    oss << message;  // Start with the base message

    // Create a string with the arguments
    int dummy[] = {0, ((void)(oss << ' ' << args), 0)...};
    (void)dummy; // Avoid unused variable warning

    // Create the log message
    std::string logMessage = std::string(timeBuf) + " [" + levelToString(level) + "] " + oss.str();

    // Write to the log file
    if (logFile.is_open()) {
        logFile << logMessage << std::endl;
    }

    // Optionally write to the console
    if (consoleOutput) {
        std::cout << logMessage << std::endl;
    }
}

#endif // LOGGER_H
