CC = gcc
CFLAGS = -I -g

client: src/client/Client.c
	mkdir -p out
	$(CC) $(CFLAGS) -o out/Client src/common/Utils.c src/common/Common.c src/client/Client.c

server: src/server/Server.c
	mkdir -p out
	$(CC) $(CFLAGS) -o out/server src/common/Utils.c src/common/Common.c src/server/Server.c

unit: test/unit/UnitTests.c
	mkdir -p out
	$(CC) $(CFLAGS) -o out/unitTest src/common/Utils.c src/common/Common.c test/unit/UnitTests.c

.PHONY: clean

clean:
	rm -rf out
