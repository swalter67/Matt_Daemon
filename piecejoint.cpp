#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <vector>
#include <string>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <cstring>

// Fonction pour encoder un fichier en base64
std::string encodeBase64(const std::string& filePath) {
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

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Pas de retour à la ligne
    BIO_write(bio, fileData.data(), fileData.size());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string encodedData(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);
    return encodedData;
}

// Corps de l'email en format MIME
std::string createMimeMessage(const std::string& attachmentPath) {
    std::string boundary = "boundary123";
    std::string base64File = encodeBase64(attachmentPath);
    std::string filename = "matt_daemon.log"; // Nom de fichier affiché dans l'email

    std::ostringstream mimeMessage;
    mimeMessage << "From: \"Matt Deamon\" <tonemail@gmail.com>\r\n"
                << "To: <destinataire@example.com>\r\n"
                << "Subject: Test Email avec Pièce Jointe\r\n"
                << "MIME-Version: 1.0\r\n"
                << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n\r\n"
                << "--" << boundary << "\r\n"
                << "Content-Type: text/plain; charset=UTF-8\r\n\r\n"
                << "Bonjour,\nCeci est un test d'envoi d'e-mail avec une pièce jointe.\r\n\r\n"
                << "--" << boundary << "\r\n"
                << "Content-Type: application/octet-stream; name=\"" << filename << "\"\r\n"
                << "Content-Disposition: attachment; filename=\"" << filename << "\"\r\n"
                << "Content-Transfer-Encoding: base64\r\n\r\n"
                << base64File << "\r\n\r\n"
                << "--" << boundary << "--\r\n";

    return mimeMessage.str();
}

// Fonction de lecture du payload
size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    std::string *payload = (std::string *)userp;
    if (payload->empty()) return 0;

    size_t copy_size = size * nmemb < payload->size() ? size * nmemb : payload->size();
    std::memcpy(ptr, payload->c_str(), copy_size);
    payload->erase(0, copy_size);
    return copy_size;
}

int main() {
    std::string attachmentPath = "/var/log/matt_daemon/matt_daemon.log"; // Nom de fichier affiché dans l'emai"; // Remplace par le chemin de ton fichier
    std::string mimeMessage = createMimeMessage(attachmentPath);

    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = nullptr;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465"); // SSL
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_USERNAME, "mattdaemonmulhouse@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "upht ymln yzoe mndc"); // ⚠️ Utiliser un mot de passe d'application
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<mattdaemonmulhouse@gmail.com>");
        recipients = curl_slist_append(recipients, "<swalter67@gmail.com>");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // Envoi de l'email en mode upload
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &mimeMessage);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Erreur d'envoi de l'e-mail : " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "E-mail avec pièce jointe envoyé avec succès !" << std::endl;
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }

    return res;
}
