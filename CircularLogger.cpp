#include "CircularLogger.h"


namespace fs = std::filesystem;
using json = nlohmann::json;

/**
 * @brief Constructor for CircularLogger. Initializes the logger by loading configuration
 * and ensuring the log directory exists.
 * @param configPath Path to the configuration JSON file.
 */
CircularLogger::CircularLogger(const std::string& configPath) : configPath(configPath) {
    loadConfig();
    ensureLogDirectory();
}

/**
 * @brief Logs a message to the current log file.
 * The function determines the correct log file based on the current time
 * and logging type. If a new log file is needed, it rotates logs accordingly.
 * @param message The message to log.
 */
void CircularLogger::log(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm timeInfo;
    localtime_s(&timeInfo, &nowTime);

    // Generate the appropriate log file name
    std::string logFileName = generateLogFileName(timeInfo);
    fs::path logFilePath = fs::path(logDirectory) / logFileName;

    //check if we need to rotate files
    if (nowTime >= nextRotationTime) {
        // Generate the appropriate log file name
        std::string logFileName = generateLogFileName(timeInfo);
        fs::path logFilePath = fs::path(logDirectory) / logFileName;
        rotateLogs();
        currentLogFile = logFilePath;
        nextRotationTime = calculateNextRotationTime(nowTime);
    }

    std::ofstream logFile(currentLogFile, std::ios::app);
    logFile << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << " - " << message << "\n";
}

/**
 * @brief Loads configuration settings from a JSON file.
 * If the file does not exist or contains invalid data, default values are used.
 */
void CircularLogger::loadConfig() {
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        saveDefaultConfig(); // Create config file with default values if missing
        return;
    }

    json configJson;
    // Load values or use defaults if missing
    try {
        configFile >> configJson;
        loggingType = configJson.value("loggingType", "second");
        frequency = configJson.value("frequency", 5);
        maxEntries = configJson.value("maxEntries", 12);
    }
    catch (...) {
        // If any error occurs, reset to default config
        saveDefaultConfig();
    }
}

/**
 * @brief Saves the default configuration settings to a JSON file.
 */
void CircularLogger::saveDefaultConfig() {
    json defaultConfig = {
        {"loggingType", "second"},
        {"frequency", 5},
        {"maxEntries", 12}
    };
    std::ofstream configFile(configPath);
    configFile << defaultConfig.dump(4);
}

/**
 * @brief Ensures that the log directory exists.
 * If the directory does not exist, it is created.
 */
void CircularLogger::ensureLogDirectory() {
    if (!fs::exists(logDirectory)) {
        fs::create_directory(logDirectory);
    }
}

/**
 * @brief Generates a log file name based on the current time.
 * The file name format is determined by the logging type.
 * @param timeInfo The current time information.
 * @return The generated log file name.
 */
std::string CircularLogger::generateLogFileName(const std::tm& timeInfo) {
    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%Y-%m-%d");
    if (loggingType == "hour") {
        oss << "-" << std::put_time(&timeInfo, "%H");
    }
    else if (loggingType == "minute") {
        oss << "-" << std::put_time(&timeInfo, "%H-%M");
    }
    else if (loggingType == "second") {
        oss << "-" << std::put_time(&timeInfo, "%H-%M-%S");
    }
    return oss.str() + ".log";
}

/**
 * @brief Calculates the next rotation time based on the current time.
 * The rotation time is determined by the logging type and frequency.
 * @param currentTime The current time as a time_t value.
 * @return The next rotation time as a time_t value.
 */
std::time_t CircularLogger::calculateNextRotationTime(std::time_t currentTime) {
    std::tm nextTime;
    localtime_s(&nextTime, &currentTime);
    if (loggingType == "hour") {
        nextTime.tm_hour += frequency;
        nextTime.tm_min = 0;
        nextTime.tm_sec = 0;
    }
    else if (loggingType == "minute") {
        nextTime.tm_min += frequency;
        nextTime.tm_sec = 0;
    }
    else if (loggingType == "second") {
        nextTime.tm_sec += frequency;
    }
    return std::mktime(&nextTime);
}

/**
 * @brief Rotates log files based on the maximum number of entries.
 * If the number of log files exceeds the maximum, the oldest files are deleted.
 */
void CircularLogger::rotateLogs() {
    std::vector<fs::path> logFiles;
    for (const auto& entry : fs::directory_iterator(logDirectory)) {
        if (entry.is_regular_file()) {
            logFiles.push_back(entry.path());
        }
    }
    // Sort files based on their last modified time (oldest first)
    std::sort(logFiles.begin(), logFiles.end(), [](const fs::path& a, const fs::path& b) {
        return fs::last_write_time(a) < fs::last_write_time(b);
        });

    // Remove oldest files if we exceed the max allowed entries
    while (logFiles.size() >= maxEntries) {
        fs::remove(logFiles.front());
        logFiles.erase(logFiles.begin());
    }
}
