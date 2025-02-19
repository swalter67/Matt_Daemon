
#include "MattDaemon.hpp"

void signalHandler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        unlink(LOCK_FILE); // Supprimer le fichier de lock
        std::ofstream logFile(LOG_DIR LOG_FILE, std::ios::app);
        if (logFile.is_open()) {
            logFile << "[INFO] Lock file deleted due to signal " << signum << "." << std::endl;
        }
        exit(signum);
    }
}


MattDaemon::MattDaemon() : lock_fd(-1), server_fd(-1) {
    // Vérifier si le fichier de lock existe déjà
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    if (access(LOCK_FILE, F_OK) == 0) {
        std::cerr << "[ERROR] Another instance of MattDaemon is already running." << std::endl;
        logger.logMessage("[ERROR]", " Another instance of MattDaemon is already running.");
        exit(EXIT_FAILURE);
    }

    daemonize();
    setupLockFile();
   
    setupServer();
}


MattDaemon::~MattDaemon() {
    logger.logMessage("[INFO]", " Daemon shutting down.");
    
    if (lock_fd != -1) {
        close(lock_fd);
        if (unlink(LOCK_FILE) == 0) {
            logger.logMessage("[INFO]"," Lock file deleted.");
        } else {
            logger.logMessage("[ERROR]" , "Failed to delete lock file.");
        }
    }

    if (server_fd != -1) {
        close(server_fd);
    }
}


void MattDaemon::daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    setsid();   //detach porcessus du terminal et pas de signaux SIGHUP pui sferme les entree sorties
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void MattDaemon::setupLockFile() {
    
    if (access("/var/lock", F_OK) == -1) {
        if (mkdir("/var/lock", 0755) == -1) {
            logger.logMessage("[ERROR]", "Unable to create /var/lock/ directory.");
            exit(EXIT_FAILURE);
        }
    }

    
    lock_fd = open(LOCK_FILE, O_CREAT | O_RDWR, 0644);
    if (lock_fd == -1) {
        logger.logMessage("[ERROR]", "Cannot create lock file in /var/lock/");
        exit(EXIT_FAILURE);
    }

    // Vérification si une autre instance tourne déjà
    if (lockf(lock_fd, F_TLOCK, 0) == -1) {
        logger.logMessage("[ERROR]"," Another instance of MattDaemon is already running.");
        std::cerr << "[ERROR] Another instance of MattDaemon is already running." << std::endl;
        exit(EXIT_FAILURE);
    }

    logger.logMessage("[INFO]", " Lock file created in /var/lock/");
}







void MattDaemon::setupServer() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        logger.logMessage("","Error: Cannot create socket.");
        exit(EXIT_FAILURE);
    }
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        logger.logMessage("","Error: Cannot bind to port.");
        exit(EXIT_FAILURE);
    }
    listen(server_fd, 3);
    logger.logMessage("INFO","Server is listening on port " + std::to_string(PORT));
}







void MattDaemon::run() {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];

    logger.logMessage("INFO", "Daemon is running and listening for connections...");

    fd_set active_fd_set, read_fd_set;
    int max_fd = server_fd;
    int client_count = 0; // Nombre de clients connectés

    FD_ZERO(&active_fd_set);
    FD_SET(server_fd, &active_fd_set);

    while (true) {
        read_fd_set = active_fd_set;

        if (select(max_fd + 1, &read_fd_set, NULL, NULL, NULL) < 0) {
            logger.logMessage("ERROR", "Select error");
            continue;
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &read_fd_set)) {
                if (fd == server_fd) {
                    // Accepter un nouveau client uniquement si on est en dessous de la limite
                    if (client_count >= 3) {
                        logger.logMessage("WARNING", "Connection refused: Maximum 3 clients reached.");
                        int temp_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                        if (temp_socket >= 0) {
                            std::string msg = "[ERROR] Maximum clients connected. Try later.\n";
                            send(temp_socket, msg.c_str(), msg.size(), 0);
                            close(temp_socket);
                        }
                        continue;
                    }

                    int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                    if (client_socket < 0) {
                        logger.logMessage("ERROR", "Failed to accept client connection.");
                        continue;
                    }

                    logger.logMessage("INFO", "New client connected.");
                    FD_SET(client_socket, &active_fd_set);
                    if (client_socket > max_fd) {
                        max_fd = client_socket;
                    }
                    client_count++; // Augmenter le nombre de clients connectés
                } else {
                    // Lire les messages du client existant
                    memset(buffer, 0, sizeof(buffer));
                    ssize_t bytes_received = read(fd, buffer, sizeof(buffer) - 1);
                    if (bytes_received <= 0) {
                        logger.logMessage("INFO", "Client disconnected.");
                        close(fd);
                        FD_CLR(fd, &active_fd_set);
                        client_count--; // Décrémenter le nombre de clients connectés
                    } else {
                        std::string message(buffer);
                        message.erase(message.find_last_not_of("\r\n") + 1);  // Supprimer les retours à la ligne

                        if (!message.empty()) {
                            logger.logMessage("LOG", "User input: " + message);

                            // Vérifie si le client envoie "quit"
                            if (message == "quit") {
                                logger.logMessage("INFO", "Received quit command. Shutting down...");
                                close(fd);
                                FD_CLR(fd, &active_fd_set);
                                client_count--;
                                shutdown(server_fd, SHUT_RDWR);

                            // a verifer si on kill ?

                                MattDaemon::~MattDaemon();
                                exit(0);
                            }

                            // Envoyer une confirmation au client
                            std::string response = "[LOGGED] " + message + "\n";
                            send(fd, response.c_str(), response.size(), 0);
                        }
                    }
                }
            }
        }
    }
}
