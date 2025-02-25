#ifndef MAIL_HPP
#define MAIL_HPP

#include <string>
#include <map>
#include <iostream>
#include "Tintin_reporter.hpp"

class Mail {
public:
    // Constructeur qui charge les paramètres depuis un fichier config.txt
   Mail(const std::string& configFile, Tintin_reporter& logger);

    // Destructeur
    ~Mail();

    // Fonction pour envoyer un email avec pièce jointe
    bool send(const std::string& recipient, const std::string& subject, const std::string& body, const std::string& attachmentPath);

private:
    // Fonction pour charger la configuration SMTP depuis un fichier
    std::map<std::string, std::string> loadConfig(const std::string& filename);

    // Fonction pour encoder un fichier en base64
    std::string encodeBase64(const std::string& filePath);

    // Générer un email MIME multipart/mixed avec une pièce jointe
    std::string createMimeMessage(const std::string& recipient, const std::string& subject, const std::string& body, const std::string& attachmentPath);

    // Fonction utilisée par cURL pour envoyer le payload
    static size_t payload_source(void* ptr, size_t size, size_t nmemb, void* userp);

    // Attributs de la classe
    std::string sender_email;      // Email de l'expéditeur
    std::string sender_password;   // Mot de passe de l'expéditeur
    std::string smtp_server;       // Serveur SMTP utilisé pour l'envoi
    std::string email_payload;     // Contenu de l'email encodé
    Tintin_reporter& logger;
};

#endif // MAIL_HPP
