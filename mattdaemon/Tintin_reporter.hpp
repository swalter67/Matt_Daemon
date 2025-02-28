#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <cstring>
#include <map>
#include <curl/curl.h>

#define LOG_DIR "/var/log/matt_daemon/"
#define LOG_FILE "matt_daemon.log"

class Tintin_reporter {
    public: 
        Tintin_reporter();
        ~Tintin_reporter();
        Tintin_reporter(const Tintin_reporter& src);
        Tintin_reporter& operator=(const Tintin_reporter &src);

        void logMessage(const std::string &level, const std::string &message);

    private:
        std::ofstream logFile;
        void openLog();
        std::string getTimestamp();    

};

#endif