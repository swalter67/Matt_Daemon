#include "Mail.hpp"
#include <iostream>

int main() {
    std::cout << "[TEST] Début du test de Mail" << std::endl;

    // Charger la configuration depuis config.txt
    Mail mail("config.txt");

    std::string recipient;
    std::cout << "[TEST] Entrez l'adresse email du destinataire : ";
    std::cin >> recipient;

    // Sujet et message de test
    std::string subject = "Test MattDaemon";
    std::string body = "Ceci est un email de test envoyé depuis le programme test_mail.";

    std::cout << "[TEST] Envoi de l'email à " << recipient << "..." << std::endl;
    bool success = mail.send(recipient, subject, body, "");

    if (success) {
        std::cout << "[TEST] ✅ Email envoyé avec succès à " << recipient << " !" << std::endl;
    } else {
        std::cerr << "[TEST] ❌ Échec de l'envoi de l'email." << std::endl;
    }

    return 0;
}
