# mattdeamon



fichier lock :/var/lock/matt_daemon.lock   supprimer a la detruction du programme

fichier log :/var/log/matt_daemon/matt_daemon.log


ps -ef | grep MattDaemon

sudo kill -15 <PID>


sudo apt install libcurl4-openssl-dev
mail : mattdaemonmulhouse@gmail.com 
t0t0Mulhouse

name Mattdaemon42mulhouse


curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465"); // SSL (465) ou TLS (587)
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_USERNAME, "mattdaemonmulhouse@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "upht ymln yzoe mndc"); // ⚠️ Utiliser un mot de passe d'application
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<mattdaemonmulhouse@gmail.com >");
        
        recipients = curl_slist_append(recipients, "<swalter67@gmail.com>");




        g++ piecejoint.cpp -o send_pj -lcurl -lssl -lcrypto




#include "Mail.hpp"


#include "Mail.hpp"

int main() {
    // Création de l'objet Mail avec le fichier de configuration
    Mail mail("config.txt");

    // Envoi d'un email avec pièce jointe
    mail.send("destinataire@example.com", "Test Email avec Pièce Jointe", 
              "Bonjour,\nCeci est un test d'envoi d'e-mail avec une pièce jointe.",
              "/var/log/matt_daemon/matt_daemon.log");

    return 0;
}

