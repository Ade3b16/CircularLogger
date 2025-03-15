#include "CircularLogger.h"


namespace fs = std::filesystem;
using json = nlohmann::json;

CircularLogger::CircularLogger(const std::string& configPath) : configPath(configPath) {
    loadConfig();
    ensureLogDirectory();
}

void CircularLogger::log(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm timeInfo;
    localtime_s(&timeInfo, &nowTime);

    std::string logFileName = generateLogFileName(timeInfo);
    fs::path logFilePath = fs::path(logDirectory) / logFileName;

    if (currentLogFile != logFileName) {
        rotateLogs();
        currentLogFile = logFileName;
    }

    std::ofstream logFile(logFilePath, std::ios::app);
    logFile << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << " - " << message << "\n";
}

void CircularLogger::loadConfig() {
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        saveDefaultConfig();
        return;
    }

    json configJson;
    try {
        configFile >> configJson;
        loggingType = configJson.value("loggingType", "second");
        frequency = configJson.value("frequency", 5);
        maxEntries = configJson.value("maxEntries", 12);
    }
    catch (...) {
        saveDefaultConfig();
    }
}

void CircularLogger::saveDefaultConfig() {
    json defaultConfig = {
        {"loggingType", "second"},
        {"frequency", 5},
        {"maxEntries", 12}
    };
    std::ofstream configFile(configPath);
    configFile << defaultConfig.dump(4);
}

void CircularLogger::ensureLogDirectory() {
    if (!fs::exists(logDirectory)) {
        fs::create_directory(logDirectory);
    }
}

std::string CircularLogger::generateLogFileName(const std::tm& timeInfo) {
    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%Y-%m-%d");
    if (loggingType == "hour") {
        oss << "-" << std::put_time(&timeInfo, "%H");
    }
    else if (loggingType == "minute") {
        oss << "-" << std::put_time(&timeInfo, "%M");
    }
    else if (loggingType == "second") {
        oss << "-" << std::put_time(&timeInfo, "%M-%S");
    }
    return oss.str() + ".log";
}

void CircularLogger::rotateLogs() {
    std::vector<fs::path> logFiles;
    for (const auto& entry : fs::directory_iterator(logDirectory)) {
        if (entry.is_regular_file()) {
            logFiles.push_back(entry.path());
        }
    }
    std::sort(logFiles.begin(), logFiles.end(), [](const fs::path& a, const fs::path& b) {
        return fs::last_write_time(a) < fs::last_write_time(b);
        });
    while (logFiles.size() > maxEntries) {
        fs::remove(logFiles.front());
        logFiles.erase(logFiles.begin());
    }
}
