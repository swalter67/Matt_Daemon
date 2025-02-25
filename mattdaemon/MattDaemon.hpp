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
#include <openssl/rsa.h>
#include <openssl/engine.h>

#include <iostream>
#include <fstream>
#include "Tintin_reporter.hpp"
#include <map>
#define LOCK_FILE "/var/lock/matt_daemon.lock"
#define LOG_DIR "/var/log/matt_daemon/"
#define LOG_FILE "matt_daemon.log"
#define PORT 4242

#define KEY_LENGTH  2048
#define PUB_EXP 3
// #define LOG
#define BEN_MSG "!ben"
#define MAX_CLIENT 3
#define BUFFER_SIZE 1024

class MattDaemon
{
    public:
        MattDaemon();
        ~MattDaemon();
        void run();

    private:
        void daemonize();
        void setupLockFile();
        void setupServer();

        void get_pub_key_from_client(int sock, char *buffer, int bsize);
        RSA *get_keys_by_sock(int sock);
        int encrypt_message(const char *message, char *buffer, RSA *pubkey);
        void decrypt_message(char *from, int fromlen, char *to);
        int new_connection(fd_set *active_fd_set);
        void send_pubkey(int sock);
        /// void signalHandler(int signum);
        int lock_fd;
        int server_fd;
        int max_fd;
        int client_count; // Nombre de clients connectés
        Tintin_reporter logger;
        #ifdef LOG
            std::ofstream      myfile;
        #endif
        RSA *keys;
        std::map<int, RSA *> *clients_keys;
};

#endif // MATTDAEMON_HPP
