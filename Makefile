CC = g++
CFLAGS = -fpermissive -Wdeprecated-declarations

SRC_MD = mattdaemon/main.cpp mattdaemon/MattDaemon.cpp mattdaemon/Tintin_reporter.cpp 
OBJ_MD = $(SRC_MD:.cpp=.o)
NAME_MD = MattDaemon

SRC_BA = benafk/main.cpp benafk/BenAFK.cpp
OBJ_BA = $(SRC_BA:.cpp=.o)
NAME_BA = BenAFK

all: $(NAME_MD) $(NAME_BA)

$(NAME_MD): $(OBJ_MD)
	$(CC) $(CFLAGS) -o $@ $(OBJ_MD) -lssl -lcrypto -fpermissive -Wdeprecated-declarations

$(NAME_BA): $(OBJ_BA)
	$(CC) $(CFLAGS) -o $@ $(OBJ_BA) -lssl -lcrypto -fpermissive -Wdeprecated-declarations

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f $(OBJ_MD) $(OBJ_BA)

fclean: clean
	rm -f $(NAME_MD) $(NAME_BA)

re: fclean all
