#include "MattDaemon.hpp"
void signalHandler(int signum)
{
    if (signum == SIGTERM || signum == SIGINT)
    {
        unlink(LOCK_FILE); // Supprimer le fichier de lock
        std::ofstream logFile(LOG_DIR LOG_FILE, std::ios::app);
        if (logFile.is_open())
        {
            logFile << "[INFO] Lock file deleted due to signal " << signum << "." << std::endl;
        }
        exit(signum);
    }
}

MattDaemon::MattDaemon() : lock_fd(-1), server_fd(-1)
{
    // Vérifier si le fichier de lock existe déjà
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGINT, signalHandler);
    if (access(LOCK_FILE, F_OK) == 0)
    {
        std::cerr << "[ERROR] Another instance of MattDaemon is already running." << std::endl;
        logger.logMessage("[ERROR]", " Another instance of MattDaemon is already running.");
        exit(EXIT_FAILURE);
    }
    this->keys = RSA_generate_key(KEY_LENGTH, PUB_EXP, NULL, NULL);
    this->clients_keys = new std::map<int, RSA *>;
    this->client_count = 0;
    #ifdef LOG
        this->myfile.open("key.me");
    #endif
    daemonize();
    setupLockFile();
    setupServer();
}

MattDaemon::~MattDaemon()
{
    logger.logMessage("[INFO]", " Daemon shutting down.");

    if (lock_fd != -1)
    {
        close(lock_fd);
        if (unlink(LOCK_FILE) == 0)
        {
            logger.logMessage("[INFO]", " Lock file deleted.");
        }
        else
        {
            logger.logMessage("[ERROR]", "Failed to delete lock file.");
        }
    }
    delete this->clients_keys;
    if (server_fd != -1)
    {
        close(server_fd);
    }
}

void MattDaemon::send_pubkey(int sock)
{
    char *key_sended;
    int key_len;

    BIO *pub = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPublicKey(pub, this->keys);
    key_len = BIO_pending(pub) + 1;
    key_sended = (char *)calloc(key_len + 1, sizeof(char));

    BIO_read(pub, key_sended, key_len);
#ifdef LOG
    myfile << key_sended << std::endl;
#endif
    int rsend = send(sock, key_sended, key_len, 0);
    free(key_sended);
    BIO_free(pub);
}

void MattDaemon::get_pub_key_from_client(int sock, char *buffer, int bsize)
{
    char *key_sended;
    int key_len;

    BIO *bio = BIO_new_mem_buf((void *)buffer, bsize);
    this->clients_keys->insert({sock, PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL)});
    BIO_free(bio);
}

RSA *MattDaemon::get_keys_by_sock(int sock)
{
    if (this->clients_keys->find(sock) != this->clients_keys->end())
        return this->clients_keys->at(sock);
#ifdef LOG
    myfile << "key not found" << std::endl;
#endif
    return NULL;
}

int MattDaemon::encrypt_message(const char *message, char *buffer, RSA *pubkey)
{
    return RSA_public_encrypt(strlen(message), (unsigned char *)message, (unsigned char *)buffer, pubkey, RSA_PKCS1_OAEP_PADDING);
}

void MattDaemon::decrypt_message(char *from, int fromlen, char *to)
{
    RSA_private_decrypt(fromlen, (unsigned char *)from, (unsigned char *)to, this->keys, RSA_PKCS1_OAEP_PADDING);
}



MattDaemon::MattDaemon(const MattDaemon  &src){
    this->lock_fd = src.lock_fd;
    this->server_fd = src.server_fd;
}

MattDaemon& MattDaemon::operator=(const MattDaemon &src){
    this->lock_fd = src.lock_fd;
    this->server_fd = src.server_fd;
    return (*this);
}

void MattDaemon::daemonize() {
    pid_t pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    setsid();   //detach porcessus du terminal et pas de signaux SIGHUP pui sferme les entree sorties
    chdir("/app");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void MattDaemon::setupLockFile()
{

    if (access("/var/lock", F_OK) == -1)
    {
        if (mkdir("/var/lock", 0755) == -1)
        {
            logger.logMessage("[ERROR]", "Unable to create /var/lock/ directory.");
            exit(EXIT_FAILURE);
        }
    }
    lock_fd = open(LOCK_FILE, O_CREAT | O_RDWR, 0644);
    if (lock_fd == -1)
    {
        logger.logMessage("[ERROR]", "Cannot create lock file in /var/lock/");
        exit(EXIT_FAILURE);
    }

    // Vérification si une autre instance tourne déjà
    if (lockf(lock_fd, F_TLOCK, 0) == -1)
    {
        logger.logMessage("[ERROR]", " Another instance of MattDaemon is already running.");
        std::cerr << "[ERROR] Another instance of MattDaemon is already running." << std::endl;
        exit(EXIT_FAILURE);
    }

    logger.logMessage("[INFO]", " Lock file created in /var/lock/");
}

void MattDaemon::setupServer()
{
    int optval = 1;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        logger.logMessage("", "Error: Cannot create socket.");
        exit(EXIT_FAILURE);
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        logger.logMessage("", "Error: Cannot bind to port.");
        exit(EXIT_FAILURE);
    }
    listen(server_fd, 3);
    this->max_fd = this->server_fd;
    logger.logMessage("INFO", "Server is listening on port " + std::to_string(PORT));
}

int MattDaemon::new_connection(fd_set *active_fd_set)
{
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int optval = 1;

    if (this->client_count >= MAX_CLIENT)
    {
        logger.logMessage("WARNING", "Connection refused: Maximum 3 clients reached.");
        int temp_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (temp_socket >= 0)
        {
            std::string msg = "[ERROR] Maximum clients connected. Try later.\n";
            send(temp_socket, msg.c_str(), msg.size(), 0);
            close(temp_socket);
        }
        return -1;
    }
    int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    if (client_socket < 0)
    {
        logger.logMessage("ERROR", "Failed to accept client connection.");
        return -1;
    }

    logger.logMessage("INFO", "New client connected.");
    FD_SET(client_socket, active_fd_set);
    if (client_socket > this->max_fd)
        this->max_fd = client_socket;
    this->client_count++; // Augmenter le nombre de clients connecté

    return 0;
}

    

void MattDaemon::run()
{

    char buffer[BUFFER_SIZE];
    char decrypted_buffer[BUFFER_SIZE];

    logger.logMessage("INFO", "Daemon is running and listening for connections...");

    fd_set active_fd_set, read_fd_set;

    FD_ZERO(&active_fd_set);
    FD_SET(server_fd, &active_fd_set);
    while (1)
    {
        read_fd_set = active_fd_set;

        if (select(max_fd + 1, &read_fd_set, NULL, NULL, NULL) < 0)
        {
            logger.logMessage("ERROR", "Select error");
            continue;
        }

        for (int fd = 0; fd <= max_fd; fd++)
        {
            if (FD_ISSET(fd, &read_fd_set))
            {
                if (fd == server_fd)
                {
                    if (this->new_connection(&active_fd_set) == 1) // Accepter un nouveau client uniquement si on est en dessous de la limite
                        continue;
                }
                else
                {
                    // Lire les messages du client existant
                    memset(buffer, 0, BUFFER_SIZE);

                    ssize_t bytes_received = read(fd, buffer, BUFFER_SIZE);
                    #ifdef LOG
                        myfile << bytes_received << ": " << buffer << std::endl;
                    #endif
                    if (bytes_received <= 0)
                    {
                        #ifdef LOG
                            myfile << "RIEN RECU" << std::endl;
                        #endif
                        this->clients_keys->erase(fd);
                        logger.logMessage("INFO", "Client disconnected.");
                        close(fd);
                        FD_CLR(fd, &active_fd_set);
                        client_count--; // Décrémenter le nombre de clients connectés
                    }
                    else
                    {
                        if (strncmp(buffer, BEN_MSG, strlen(BEN_MSG)) == 0)
                        {
                            #ifdef LOG
                                myfile << "Ben client" << std::endl;
                            #endif
                            this->get_pub_key_from_client(fd, buffer + 4, bytes_received - 4);
                            this->send_pubkey(fd);
                            #ifdef LOG
                                myfile << "Ben Auth finish" << std::endl;
                            #endif
                            continue;
                        }
                        RSA *pubkey = this->get_keys_by_sock(fd);
                        if (pubkey)
                        {
                            memset(decrypted_buffer, 0, BUFFER_SIZE);
                            this->decrypt_message(buffer, bytes_received, decrypted_buffer);
                            memset(buffer, 0, BUFFER_SIZE);
                            strncpy(buffer, decrypted_buffer, strlen(decrypted_buffer));
                            
                        }
                        
                        std::string message(buffer);
                        message.erase(message.find_last_not_of("\r\n") + 1);  // Supprimer les retours à la ligne
                        #ifdef LOG
                            myfile << "[" << message << "]" << std::endl;
                        #endif
                        if (!message.empty()) {
                            logger.logMessage("LOG", "User input: " + message);

                            // Vérifie si le client envoie "quit"

                            if (message.rfind("email ", 0) == 0) {  // Vérifie si le message commence par "email "
                                std::string recipient = message.substr(6);  // Extrait l'adresse email après "email "

                                if (recipient.empty()) {
                                    std::string response = "[ERROR] Format incorrect. Utilisation : email destinataire@example.com\n";
                                    send(fd, response.c_str(), response.size(), 0);
                                    continue;
                                }

                                logger.logMessage("DEBUG", "Avant instanciation de Mail");

                                Mail mail(logger);  // Charge la config

                                logger.logMessage("DEBUG", "Après instanciation de Mail");

                                // Envoi du mail avec un sujet et un message par défaut
                                bool success = mail.send(recipient, "MattDaemon Notification", "Ceci est un email automatique envoyé par MattDaemon.", "/var/log/matt_daemon/matt_daemon.log");

                                if (success) {
                                    std::string response = "[INFO] Email envoyé à " + recipient + "\n";
                                    send(fd, response.c_str(), response.size(), 0);
                                    logger.logMessage("INFO", "Email envoyé avec succès à " + recipient);
                                }
                                else{
                                    std::string response = "[ERROR] Échec de l'envoi de l'email.\n";
                                    send(fd, response.c_str(), response.size(), 0);
                                    logger.logMessage("ERROR", "Échec de l'envoi de l'email.");
                                }
                            }
                            if (message == "quit") {
                                logger.logMessage("INFO", "Received quit command. Shutting down...");
                                close(fd);
                                FD_CLR(fd, &active_fd_set);
                                client_count--;
                                shutdown(server_fd, SHUT_RDWR);
                                return ;
                            }

                            // Envoyer une confirmation au client
                            std::string response = "[LOGGED] " + message + "\n";
                            if (pubkey)
                            {
                                memset(buffer, 0, BUFFER_SIZE);
                                int lsend = this->encrypt_message(response.c_str(), buffer, pubkey);
                                send(fd, buffer, lsend, 0);
                                memset(buffer, 0, BUFFER_SIZE);
                            }
                            else
                                send(fd, response.c_str(), response.size(), 0);
                        }
                    }
                }
            }
        }
    }
}
