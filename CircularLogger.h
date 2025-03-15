#pragma once

#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <thread>

class CircularLogger {
public:
    CircularLogger(const std::string& configPath = "config.json");//default constructor
    void log(const std::string& message);

private:
    std::string configPath;
    std::string loggingType;
    int frequency;
    int maxEntries;
    std::string logDirectory = "Logs";
    std::string currentLogFile;

    void loadConfig();
    void saveDefaultConfig();
    void ensureLogDirectory();
    std::string generateLogFileName(const std::tm& timeInfo);
    void rotateLogs();
};

