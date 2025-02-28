#include "MattDaemon.hpp"

static void signalHandler(int signum) {

    if (signum == SIGTERM || signum == SIGINT) {
        unlink(LOCK_FILE); // Supprimer le fichier de lock
        std::ofstream logFile(LOG_DIR LOG_FILE, std::ios::app);
        if (logFile.is_open())
        {
            logFile << "[INFO] Lock file deleted due to signal " << signum << "." << std::endl;
            logFile.close();
        }
        exit(signum);
    }
}

MattDaemon::MattDaemon() : lock_fd(-1), server_fd(-1), keys(NULL), client_count(NULL) {

    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGINT, signalHandler);
    this->clients_keys = new std::map<int, RSA *>;
    if (access(LOCK_FILE, F_OK) == 0)
        this->throw_err("Another instance of MattDaemon is already running.");
    #ifdef LOG
        this->myfile.open("key.me");
    #endif
    this->setupServer();
    this->daemonize();
    this->setupLockFile();
    this->keys = RSA_generate_key(KEY_LENGTH, PUB_EXP, NULL, NULL);
    
}

MattDaemon::~MattDaemon() {

    logger.logMessage("INFO", " Daemon shutting down.");

    if (this->lock_fd > 0) {
        close(this->lock_fd);
        if (unlink(LOCK_FILE) == 0)
            logger.logMessage("INFO", " Lock file deleted.");
        else
            logger.logMessage("ERROR", "Failed to delete lock file.");
    }
    delete this->clients_keys;
    if (server_fd != -1)
        close(server_fd);
}

MattDaemon::MattDaemon(const MattDaemon  &src){
    this->lock_fd = src.lock_fd;
    this->server_fd = src.server_fd;
    this->client_count = src.client_count;
}

MattDaemon& MattDaemon::operator=(const MattDaemon &src){
    this->lock_fd = src.lock_fd;
    this->server_fd = src.server_fd;
    return (*this);
}

void MattDaemon::throw_err(const std::string& err_msg) {

    logger.logMessage("ERROR", err_msg);
    throw std::runtime_error("[ERROR] " + err_msg);
}

void MattDaemon::exchange_pub_key(int sock, char *buffer, int bsize) {

    char *key_sended;
    int key_len;
    #ifdef LOG
        myfile << "Ben client" << std::endl;
    #endif
    BIO *bio = BIO_new_mem_buf((void *)buffer, bsize);
    this->clients_keys->insert({sock, PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL)});
    
    BIO *pub = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPublicKey(pub, this->keys);
    key_len = BIO_pending(pub) + 1;
    key_sended = (char *)calloc(key_len + 1, sizeof(char));

    BIO_read(pub, key_sended, key_len);
    #ifdef LOG
        myfile << key_sended << std::endl;
    #endif
    send(sock, key_sended, key_len, 0);
    free(key_sended);
    BIO_free(bio);
    BIO_free(pub);
    #ifdef LOG
        myfile << "Ben Auth finish" << std::endl;
    #endif
}


RSA *MattDaemon::get_keys_by_sock(int sock) {

    if (this->clients_keys->find(sock) != this->clients_keys->end())
        return this->clients_keys->at(sock);
#ifdef LOG
    myfile << "key not found" << std::endl;
#endif
    return NULL;
}

int MattDaemon::encrypt_message(const char *message, char *buffer, RSA *pubkey) {

    return RSA_public_encrypt(strlen(message), (unsigned char *)message, (unsigned char *)buffer, pubkey, RSA_PKCS1_OAEP_PADDING);
}

void MattDaemon::decrypt_message(char *from, int fromlen, char *to) {

    RSA_private_decrypt(fromlen, (unsigned char *)from, (unsigned char *)to, this->keys, RSA_PKCS1_OAEP_PADDING);
}


void MattDaemon::daemonize() {

    pid_t pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    setsid();
    // chdir("/app");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void MattDaemon::setupLockFile() {

    if (access("/var/lock", F_OK) == -1)
        if (mkdir("/var/lock", 0755) == -1)
            this->throw_err("Unable to create /var/lock/ directory.");
    this->lock_fd = open(LOCK_FILE, O_CREAT | O_RDWR, 0644);
    if (this->lock_fd == -1)
        this->throw_err("Cannot create lock file in /var/lock/");
    if (lockf(this->lock_fd, F_TLOCK, 0) == -1)
        this->throw_err("Another instance of MattDaemon is already running.");
    logger.logMessage("INFO", " Lock file created in /var/lock/");
}

void MattDaemon::setupServer() {

    sockaddr_in server_addr;
    const int optval = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        this->throw_err("Cannot create socket.");
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        this->throw_err("Cannot bind to port.");
    listen(server_fd, 3);
    this->max_fd = this->server_fd;
    logger.logMessage("INFO", "Server is listening on port " + std::to_string(PORT));
}

void MattDaemon::new_connection(fd_set *active_fd_set) {

    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    const int optval = 1;

    if (this->client_count >= MAX_CLIENT) {
        logger.logMessage("WARNING", "Connection refused: Maximum 3 clients reached.");
        int temp_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (temp_socket >= 0)
        {
            std::string msg = "[ERROR] Maximum clients connected. Try later.\n";
            send(temp_socket, msg.c_str(), msg.size(), 0);
            close(temp_socket);
        }
        return ;
    }
    int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    if (client_socket < 0) {
        logger.logMessage("ERROR", "Failed to accept client connection.");
        return ;
    }

    logger.logMessage("INFO", "New client connected.");
    FD_SET(client_socket, active_fd_set);
    if (client_socket > this->max_fd)
        this->max_fd = client_socket;
    this->client_count++; // Augmenter le nombre de clients connecté

}

void MattDaemon::disconnected_client(int fd, fd_set *active_fd_set) {
    #ifdef LOG
        myfile << "DISCONNECTED CLIENT" << std::endl;
    #endif
    this->clients_keys->erase(fd);
    logger.logMessage("INFO", "Client disconnected.");
    close(fd);
    FD_CLR(fd, active_fd_set);
    this->client_count--; // Décrémenter le nombre de clients connectés
}

void MattDaemon::mail_process(int fd, std::string recipient) {
    if (recipient.empty()) {
        std::string response = "[ERROR] Format incorrect. Utilisation : email destinataire@example.com\n";
        send(fd, response.c_str(), response.size(), 0);
        return ;
    }
    logger.logMessage("DEBUG", "Avant instanciation de Mail");
    Mail mail(logger);
    logger.logMessage("DEBUG", "Après instanciation de Mail");
    if (mail.send(recipient, "MattDaemon Notification", "Ceci est un email automatique envoyé par MattDaemon.", "/var/log/matt_daemon/matt_daemon.log")) {
        std::string response = "[INFO] Email envoyé à " + recipient + "\n";
        send(fd, response.c_str(), response.size(), 0);
        logger.logMessage("INFO", "Email envoyé avec succès à " + recipient);
    }
    else
    {
        std::string response = "[ERROR] Échec de l'envoi de l'email.\n";
        send(fd, response.c_str(), response.size(), 0);
        logger.logMessage("ERROR", "Échec de l'envoi de l'email.");
    }
}

void MattDaemon::quit_process(int fd,  fd_set *active_fd_set) {

    logger.logMessage("INFO", "Received quit command. Shutting down...");
    close(fd);
    FD_CLR(fd, active_fd_set);
    this->client_count--;
    shutdown(server_fd, SHUT_RDWR);
}

void MattDaemon::process_buffer(char *buffer, int fd, char *decrypted_buffer, fd_set *active_fd_set) {
    
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_received = read(fd, buffer, BUFFER_SIZE);
    
    #ifdef LOG
        myfile << bytes_received << ": " << buffer << std::endl;
    #endif
    if (bytes_received <= 0)
        this->disconnected_client(fd, active_fd_set);
    else
    {
        if (strncmp(buffer, BEN_MSG, strlen(BEN_MSG)) == 0)
        {
            this->exchange_pub_key(fd, buffer + 4, bytes_received - 4);
            return;
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
        message.erase(message.find_last_not_of("\r\n") + 1);
        #ifdef LOG
            myfile << "[" << message << "]" << std::endl;
        #endif
        if (!message.empty())
        {
            logger.logMessage("LOG", "User input: " + message);
            if (message.rfind("email ", 0) == 0)
                this->mail_process(fd, message.substr(6));
            else if (message == "quit")
            {
                this->quit_process(fd,  active_fd_set);
                return ;
            }
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

void MattDaemon::run() {
    char    buffer[BUFFER_SIZE];
    char    decrypted_buffer[BUFFER_SIZE];
    fd_set  active_fd_set, read_fd_set;

    logger.logMessage("INFO", "Daemon is running and listening for connections...");

    FD_ZERO(&active_fd_set);
    FD_SET(server_fd, &active_fd_set);
    while (1) {
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
                    this->new_connection(&active_fd_set); // Accepter un nouveau client uniquement si on est en dessous de la limite
                else
                    this->process_buffer(buffer, fd, decrypted_buffer, &active_fd_set);
            }
        }
    }
}
