#include "BenAFK.hpp"

void BenAFK::send_pubkey(void)
{
    char *key_sended;
    int key_len;

    BIO *pub = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPublicKey(pub, this->keys);
    key_len = BIO_pending(pub) + 1;
    key_sended = (char *)calloc(key_len, sizeof(char));

    BIO_read(pub, key_sended, key_len);
    // #ifdef PRINT_KEYS
    //     printf("\nben key: %s\n", key_sended);
    // #endif
    std::string sended(BEN_MSG);
    sended.append(key_sended);
    send(this->sock, sended.c_str(), sended.size(), 0);
    printf("key has been send\n");
    free(key_sended);
    BIO_free(pub);
}

void BenAFK::get_pubkey()
{
    char key_readed[KEY_LENGTH];

    memset(key_readed, 0, KEY_LENGTH);
    int readoc = recv(this->sock, key_readed, KEY_LENGTH, 0);
    BIO *bio = BIO_new_mem_buf((void *)key_readed, readoc);
    this->pub_matt = PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    // #ifdef PRINT_KEYS
    // BIO *pub2 = BIO_new(BIO_s_mem());
    // PEM_write_bio_RSAPublicKey(pub2, this->pub_matt);
    // int key_len = BIO_pending(pub2);
    // char *pub_key = (char *)calloc(key_len + 1, 1);
    // BIO_read(pub2, pub_key, key_len);

    // pub_key[key_len] = '\0';
    // printf("len :%d [%s]", key_len, pub_key);
    // #endif
}

int BenAFK::encrypt_message(const char *from, int fromlen, char *to)
{
    int len = RSA_public_encrypt(fromlen, (unsigned char *)from, (unsigned char *)to, this->pub_matt, RSA_PKCS1_OAEP_PADDING);
    if (len < 0)
        perror("encryption");
    return len;
}

void BenAFK::decrypt_message(char *from, int fromlen, char *to)
{
    if (RSA_private_decrypt(fromlen, (unsigned char *)from, (unsigned char *)to, this->keys, RSA_PKCS1_OAEP_PADDING) < 0)
        perror("decryption");
}

BenAFK::BenAFK()
{
    this->pub_matt = NULL;
    int optval = 1;
    this->keys = RSA_generate_key(KEY_LENGTH, PUB_EXP, NULL, NULL);

    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sock == -1)
    {
        perror("socket creation");
        exit(EXIT_FAILURE);
    }
    setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (connect(this->sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("binding socket");
        exit(EXIT_FAILURE);
    }
    this->send_pubkey();
    this->get_pubkey();
    for (int i = 0; i < MAX_DISAPLY; i++)
    {
        this->log[i][0] = '\n';
        memset(&this->log[i][1], 0, BUFFER_SIZE - 1);
    }
}

BenAFK::BenAFK(const BenAFK &src)
{
    this->sock = src.sock;
}

BenAFK &BenAFK::operator=(const BenAFK &src)
{
    this->sock = src.sock;
    return (*this);
}

BenAFK::~BenAFK()
{
    close(sock);
}

void BenAFK::add_to_log(char *log)
{
    for (int i = 0; i < MAX_DISAPLY - 1; i++)
        strcpy(this->log[i], this->log[i + 1]);
    strcpy(this->log[MAX_DISAPLY - 1], log);
}

void BenAFK::display()
{
    system("clear");
    for (int i = 0; i < MAX_DISAPLY; i++)
        std::cout << this->log[i];
}

void BenAFK::run() {
   
    char    to_send[BUFFER_SIZE];
    char    rbuffer[BUFFER_SIZE];
    char    msg[KEY_LENGTH];

    memset(rbuffer, 0, BUFFER_SIZE);
    while (1)
    {
        this->display();
        memset(msg, 0, KEY_LENGTH);
        printf("Message: ");
        fgets(msg, KEY_LENGTH, stdin);
        if (strlen(msg) > 0)
        {
            memset(to_send, 0, BUFFER_SIZE);
            msg[strlen(msg)-1] = '\0';
            int len = this->encrypt_message(msg, strlen(msg), to_send);
            send(this->sock, to_send, len, 0);
            int rlen = recv(this->sock, rbuffer, BUFFER_SIZE, O_NONBLOCK);
            if (rlen > 0)
            {
                memset(msg, 0, KEY_LENGTH);
                this->decrypt_message(rbuffer, rlen, msg);
                this->add_to_log(msg);
            }
            if (strcmp(msg, "quit") == 0)
                break;
            memset(msg, 0, BUFFER_SIZE);
        }
    }
}