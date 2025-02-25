#include <iostream>
#include <cstring>
#include <curl/curl.h>
#include <string.h>
#include <cstddef>

static const char *payload_text[] = {
    "To: destinataire@example.com\r\n",
    "From: expéditeur@example.com\r\n",
    "Subject: Test Email via libcurl\r\n",
    "\r\n",
    "Bonjour,\n\nCeci est un email envoyé avec libcurl en C++.\n",
    NULL
};

struct upload_status {
    int lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data = payload_text[upload_ctx->lines_read];

    if (data) {
        size_t len = strlen(data);
        memcpy(ptr, data, len);
        upload_ctx->lines_read++;
        return len;
    }
    return 0;
}

void sendEmail() {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx = { 0 };

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, "tonemail@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "ton_mot_de_passe");
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<tonemail@gmail.com>");
        recipients = curl_slist_append(recipients, "<destinataire@example.com>");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            std::cerr << "Erreur d'envoi d'email: " << curl_easy_strerror(res) << std::endl;

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
}

int main() {
    sendEmail();
    return 0;
}

