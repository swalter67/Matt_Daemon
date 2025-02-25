#include "Mail.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <cstring>
#include <fstream>
#include <map>
#include <iostream>
#include <curl/curl.h>
#

Mail::Mail(const std::string& configFile, Tintin_reporter& logger) : logger(logger) {
// Constructeur
logger.logMessage("DEBUG", "Chargement de la configuration depuis " + configFile);
    //std::map<std::string, std::string> config = loadConfig(configFile);
    
    //if (config.empty()) {
    //    logger.logMessage( "Erreur" , ": Impossible de charger les paramètres SMTP depuis " );
    //} else {
        sender_email = "mattdaemonmulhouse@gmail.com";
        sender_password = "upht ymln yzoe mndc";
        smtp_server = "smtp://smtp.gmail.com:587";
    //}
}

// Destructeur
Mail::~Mail() {}

// Fonction pour encoder un fichier en base64
std::string Mail::encodeBase64(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier " << filePath << std::endl;
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string fileData = ss.str();

    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, fileData.data(), fileData.size());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string encodedData(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return encodedData;
}

// Générer un email MIME avec une pièce jointe
std::string Mail::createMimeMessage(const std::string& recipient, const std::string& subject, const std::string& body, const std::string& attachmentPath) {
    std::string boundary = "boundary123";
    std::string base64File = encodeBase64(attachmentPath);
    std::string filename = attachmentPath.substr(attachmentPath.find_last_of('/') + 1);

    std::ostringstream mimeMessage;
    mimeMessage << "From: " << "mattdeamonmulhouse@gail.com" << "\r\n"
                << "To: " << recipient << "\r\n"
                << "Subject: " << subject << "\r\n"
                << "MIME-Version: 1.0\r\n"
                << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n\r\n"
                << "--" << boundary << "\r\n"
                << "Content-Type: text/plain; charset=UTF-8\r\n\r\n"
                << body << "\r\n\r\n"
                << "--" << boundary << "\r\n"
                << "Content-Type: application/octet-stream; name=\"" << filename << "\"\r\n"
                << "Content-Disposition: attachment; filename=\"" << filename << "\"\r\n"
                << "Content-Transfer-Encoding: base64\r\n\r\n"
                << base64File << "\r\n\r\n"
                << "--" << boundary << "--\r\n";

    return mimeMessage.str();
}

// Fonction de lecture du payload
size_t Mail::payload_source(void* ptr, size_t size, size_t nmemb, void* userp) {
    std::string* payload = static_cast<std::string*>(userp);
    if (payload->empty()) return 0;

    size_t copy_size = size * nmemb < payload->size() ? size * nmemb : payload->size();
    memcpy(ptr, payload->c_str(), copy_size);
    payload->erase(0, copy_size);
    return copy_size;
}

// Fonction d'envoi de l'e-mail
bool Mail::send(const std::string& recipient, const std::string& subject, const std::string& body, const std::string& attachmentPath) {
    CURL* curl;
    CURLcode res = CURLE_OK;
    struct curl_slist* recipients = nullptr;

    // Vérifier que les paramètres SMTP sont chargés
    if (sender_email.empty() || sender_password.empty() || smtp_server.empty()) {
        std::cerr << "Erreur : Les paramètres SMTP ne sont pas correctement chargés." << std::endl;
        return false;
    }

    // Générer le message MIME avec la pièce jointe
    email_payload = createMimeMessage(recipient, subject, body, attachmentPath);

    // Initialiser cURL
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Erreur : Impossible d'initialiser cURL." << std::endl;
        return false;
    }

    // Configuration du serveur SMTP
    curl_easy_setopt(curl, CURLOPT_URL, smtp_server.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

    // Authentification SMTP
    curl_easy_setopt(curl, CURLOPT_USERNAME, sender_email.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, sender_password.c_str());

    // Adresse de l'expéditeur
    std::string mail_from = "<" + sender_email + ">";
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mail_from.c_str());

    // Ajout du destinataire
    std::string mail_to = "<" + recipient + ">";
    recipients = curl_slist_append(recipients, mail_to.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    // Configuration de l'upload du message
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &email_payload);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    // Exécution de l'envoi
    res = curl_easy_perform(curl);

    // Nettoyage
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        logger.logMessage( "log",  "Erreur d'envoi de l'e-mail : \n" );
        return false;
    }

    logger.logMessage( "log",  "envoi de l'e-mail : ok\n" );
    return true;
}

// Fonction pour charger les paramètres depuis un fichier `config.txt`
std::map<std::string, std::string> Mail::loadConfig(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);

    if (!file) {
        logger.logMessage("ERROR", "Impossible d'ouvrir le fichier de configuration : " + filename);
        return config; // Retourne un map vide
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignorer les lignes vides
        if (line.empty()) continue;

        // Vérifier si la ligne contient un '=' pour séparer clé et valeur
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Supprimer les espaces éventuels autour des valeurs
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n\""));  // Supprime les guillemets éventuels
            value.erase(value.find_last_not_of(" \t\r\n\"") + 1);  // Supprime les guillemets éventuels

            config[key] = value;
            
            // 🔹 Logger chaque clé et valeur ajoutée
            logger.logMessage("DEBUG", "Paramètre chargé : " + key + " = " + value);
        } else {
            logger.logMessage("WARNING", "Ligne incorrecte ignorée dans config.txt : " + line);
        }
    }

    file.close();

    if (config.empty()) {
        logger.logMessage("ERROR", "Le fichier de configuration est vide ou mal formaté.");
    } else {
        logger.logMessage("INFO", "Fichier de configuration chargé avec succès.");
    }

    return config;
}
