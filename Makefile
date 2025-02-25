# Compilateur
CC = g++ 
CFLAGS = -Wno-deprecated-declarations 
LIBS = -lcurl -lssl -lcrypto
SRC_MD = mattdaemon/main.cpp mattdaemon/MattDaemon.cpp mattdaemon/Tintin_reporter.cpp mattdaemon/Mail.cpp
OBJ_MD = $(SRC_MD:.cpp=.o)
NAME_MD = MattDaemon

SRC_BA = benafk/main.cpp benafk/BenAFK.cpp
OBJ_BA = $(SRC_BA:.cpp=.o)
NAME_BA = BenAFK


all: $(NAME_MD) $(NAME_BA)

$(NAME_MD): $(OBJ_MD)
	$(CC) $(CFLAGS) -o $@ $(OBJ_MD) $(LIBS)

$(NAME_BA): $(OBJ_BA)
	$(CC) $(CFLAGS) -o $@ $(OBJ_BA) $(LIBS)


# Compilation des fichiers objets
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers objets
clean:
	rm -f $(OBJ_MD) $(OBJ_BA)

# Nettoyage complet
fclean: clean
	rm -f $(NAME_MD) $(NAME_BA)

# Recompilation complète
re: fclean all


