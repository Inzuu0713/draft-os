# Makefile — 5x6 Chess C project
#
# Usage:
#   make server        — compile server (default port 12345)
#   ./bin/server 5000  — run server on custom port
#   make client        — compile and run the client
#   make play          — compile and run local single-machine game
#   make clean         — remove compiled files

CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -I include
SRC    = src/board.c src/moves.c src/game.c

.PHONY: server client play clean

server: bin/server
	./bin/server $(PORT)

bin/server: $(SRC) src/server.c | bin
	$(CC) $(CFLAGS) $(SRC) src/server.c -o bin/server

client: bin/client
	./bin/client

bin/client: $(SRC) src/client.c | bin
	$(CC) $(CFLAGS) $(SRC) src/client.c -o bin/client

play: bin/play
	./bin/play

bin/play: $(SRC) tests/play.c | bin
	$(CC) $(CFLAGS) $(SRC) tests/play.c -o bin/play

bin:
	mkdir -p bin

clean:
	rm -rf bin
