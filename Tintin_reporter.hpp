#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>

#define LOG_DIR "/var/log/matt_daemon/"
#define LOG_FILE "matt_daemon.log"

class Tintin_reporter {
    public: 
        Tintin_reporter();
        ~Tintin_reporter();
        void logMessage(const std::string &level, const std::string &message);

    private:
        std::ofstream logFile;
        void openLog();
        std::string getTimestamp();    

};








#endif