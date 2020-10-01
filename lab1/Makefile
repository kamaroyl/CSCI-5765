CC = gcc
CFLAGS = -I -g

client: client.c
	$(CC) $(CFLAGS) -o client Utils.c Common.c Client.c

server: Server.c
	$(CC) $(CFLAGS) -o server Utils.c Common.c Server.c

.PHONY: clean

clean:
	rm -rf client; rm -rf server