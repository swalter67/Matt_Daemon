#ifndef BENAFK_HPP
# define BENAFK_HPP

# include "../mattdaemon/MattDaemon.hpp"
# include <string>

class BenAFK {
    private:
        int sock;
    public:
        BenAFK();
        BenAFK(const BenAFK& src);
        BenAFK& operator=(const BenAFK& src);
        ~BenAFK();
        void run();
};

#endif
