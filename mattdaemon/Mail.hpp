#ifndef MAIL_HPP
#define MAIL_HPP

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
#include "Tintin_reporter.hpp"

class Mail {
public:
    Mail();
    Mail( Tintin_reporter& logger);
    ~Mail();
    bool send(const std::string& recipient, const std::string& subject, const std::string& body, const std::string& attachmentPath);

    Mail(const Mail& src);
    Mail& operator=(const Mail &src);

private:
    
    
    std::string encodeBase64(const std::string& filePath);
    std::string createMimeMessage(const std::string& recipient, const std::string& subject, const std::string& body, const std::string& attachmentPath);
    static size_t payload_source(void* ptr, size_t size, size_t nmemb, void* userp);

    std::string sender_email;      // Email de l'expéditeur
    std::string sender_password;   // Mot de passe de l'expéditeur
    std::string smtp_server;       // Serveur SMTP utilisé pour l'envoi
    std::string email_payload;     // Contenu de l'email encodé
    Tintin_reporter logger;
};

#endif // MAIL_HPP
