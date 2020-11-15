CC = gcc
CFLAGS = -I -g

.PHONY: clean fileTest

linkStateRouting:
	mkdir -p out
	$(CC) $(CFLAGS) -o out/linkStateRouting src/Graph.c src/MinHeap.c src/FileReader.c src/Main.c

fileTest:
	mkdir -p out
	$(CC) $(CFLAGS) -o out/fileTest test/FileReaderTest.c src/Graph.c src/MinHeap.c src/FileReader.c

clean:
	rm -rf out
