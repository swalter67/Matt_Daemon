#include "Tintin_reporter.hpp"
#include <sys/stat.h>
#include <iomanip>

Tintin_reporter::Tintin_reporter(){
    openLog();
}

Tintin_reporter::~Tintin_reporter(){
    if(logFile.is_open()) {
        logFile.close();
    }
}

void Tintin_reporter::openLog(){
    mkdir(LOG_DIR, 0755);

    logFile.open(std::string(LOG_DIR) + LOG_FILE, std::ios::app);
    if(!logFile.is_open()){
        std::cerr << "[ERROR] Unable to open log file!" << std::endl;

    }
}

std::string Tintin_reporter::getTimestamp() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "[%d/%m/%Y - %H:%M:%S ]", timeinfo);
    return std::string(buffer);
}

void Tintin_reporter::logMessage(const std::string &level, const std::string &message) {
    if (logFile.is_open()) {
        logFile << getTimestamp() << " [" << level << "] " << message << std::endl;
    } else {
        std::cerr << "[ERROR] Log file not open!" << std::endl;
    }
}