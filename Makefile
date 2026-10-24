CC = gcc
CFLAGS = -Wall -Wextra -g

.PHONY: all clean

all: bin/server bin/client

SERVER_SRC = server/src/server.c server/src/utils/log.c
SERVER_OBJ = $(SERVER_SRC:server/src/%.c=server/build/%.o)

CLIENT_SRC = client/src/client.c client/src/utils/log.c
CLIENT_OBJ = $(CLIENT_SRC:client/src/%.c=client/build/%.o)

clean:
	rm -rf server/build client/build bin

server/build/%.o: server/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Iserver/include -c $< -o $@

client/build/%.o: client/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Iclient/include -c $< -o $@

bin/server: $(SERVER_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

bin/client: $(CLIENT_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@