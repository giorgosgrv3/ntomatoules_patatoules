CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

CLIENT_TARGET = client
SERVER_TARGET = server

CLIENT_SRC = client.c
SERVER_SRC = server.c

.PHONY: all clean

all: $(CLIENT_TARGET) $(SERVER_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRC)

$(SERVER_TARGET): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_SRC)

clean:
	rm -f $(CLIENT_TARGET) $(SERVER_TARGET)
