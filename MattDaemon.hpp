#ifndef MATTDAEMON_HPP
#define MATTDAEMON_HPP

#include <iostream>
#include <fstream>
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctime> 
#include <signal.h>

#include "Tintin_reporter.hpp"

#define LOCK_FILE "/var/lock/matt_daemon.lock"
#define LOG_DIR "/var/log/matt_daemon/"
#define LOG_FILE "matt_daemon.log"
#define PORT 4242

class MattDaemon {
public:
    MattDaemon();
    ~MattDaemon();
    void run();

private:
    void daemonize();
    void setupLockFile();
    void setupServer();
    
    ///void signalHandler(int signum);
    int lock_fd;
    int server_fd;
    Tintin_reporter logger;
};

#endif // MATTDAEMON_HPP
