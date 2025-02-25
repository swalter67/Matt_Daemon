# Compilateur
CC = g++
CFLAGS = -Wall -Wextra -Werror  -I.  # Ajout de -I. pour les headers

# Fichiers source et objets
SRC = main.cpp MattDaemon.cpp Tintin_reporter.cpp Mail.cpp
OBJ = $(SRC:.cpp=.o)

# Nom de l'exécutable
EXEC = MattDaemon

# Libs nécessaires
LIBS = -lcurl -lssl -lcrypto

# Règle principale
all: $(EXEC)

# Compilation de l'exécutable
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ) $(LIBS)

# Compilation des fichiers objets
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers objets
clean:
	rm -f $(OBJ)

# Nettoyage complet
fclean: clean
	rm -f $(EXEC)

# Recompilation complète
re: fclean all

test_mail: test_mail.cpp Mail.o
	$(CC) $(CFLAGS) -o test_mail test_mail.cpp Mail.o -lcurl -lssl -lcrypto
