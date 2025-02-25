#ifndef BENAFK_HPP
# define BENAFK_HPP

# include "../mattdaemon/MattDaemon.hpp"

# define MAX_DISAPLY 50

class BenAFK {
    private:
        int         sock;
        RSA         *keys;
        RSA         *pub_matt;
        char        log[MAX_DISAPLY][BUFFER_SIZE];
    public:
        BenAFK();
        BenAFK(const BenAFK& src);
        BenAFK& operator=(const BenAFK& src);
        ~BenAFK();
        void display();
        void run(void);
        void get_pubkey();
        void add_to_log(char *log);
        int encrypt_message(const char *from, int fromlen, char *to);
        void decrypt_message(char* from , int fromlen, char *to);
        void send_pubkey(void);
};

#endif