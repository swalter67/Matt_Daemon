
#include <iostream>
#include <curl/curl.h>
#include <cstring>

const char *payload_text =
    "From: \"Stéphane Walter\" <tonemail@gmail.com>\r\n"
    "To: <destinataire@example.com>\r\n"
    "Subject: Test Email via cURL\r\n"
    "\r\n"
    "Bonjour,\r\n"
    "Ceci est un test d'envoi d'e-mail via cURL en C++.\r\n"
    "Cordialement,\r\n"
    "Stéphane\r\n";

size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    const char **payload = (const char **)userp;
    if (*payload) {
        size_t len = strlen(*payload);
        size_t copy_size = size * nmemb < len ? size * nmemb : len;
        memcpy(ptr, *payload, copy_size);
        *payload += copy_size;
        return copy_size;
    }
    return 0;
}

int main() {
    CURL *curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *recipients = NULL;

        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465"); // SSL (465) ou TLS (587)
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_USERNAME, "mattdaemonmulhouse@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "upht ymln yzoe mndc"); // ⚠️ Utiliser un mot de passe d'application
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<mattdaemonmulhouse@gmail.com >");
        
        recipients = curl_slist_append(recipients, "<swalter67@gmail.com>");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &payload_text);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Erreur d'envoi de l'e-mail : " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "E-mail envoyé avec succès !" << std::endl;
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }

    return res;
}
