#include "CircularLogger.h"

int main() {
    CircularLogger logger;
    logger.log("This is a test log entry.");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    logger.log("This is a test log entry.");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    logger.log("This is a test log entry.");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    logger.log("This is a test log entry.");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    logger.log("This is a test log entry.");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    logger.log("This is a test log entry.");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    logger.log("This is a test log entry.");
    return 0;
}