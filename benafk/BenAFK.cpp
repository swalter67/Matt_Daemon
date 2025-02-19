#include "BenAFK.hpp"


BenAFK::BenAFK() {
    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sock == -1) {
        perror("socket creation");
        exit(EXIT_FAILURE);
    }
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_LOOPBACK;
    server_addr.sin_port = htons(PORT);
    if (bind(this->sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("binding socket");
        exit(EXIT_FAILURE);
    }
};

BenAFK::BenAFK(const BenAFK& src) {
    this->sock = src.sock;
};

BenAFK& BenAFK::operator=(const BenAFK& src) {
    this->sock = src.sock;
    return (*this);
};

BenAFK::~BenAFK(){
    close(sock);
};

void BenAFK::run() {



};