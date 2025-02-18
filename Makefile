CC = g++
CFLAGS = -Wall -Wextra -Werror -std=c++11
SRC = main.cpp MattDaemon.cpp Tintin_reporter.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = MattDaemon

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(EXEC)

re: fclean all
