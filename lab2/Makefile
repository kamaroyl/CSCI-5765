CC = gcc
CFLAGS = -I -g -O3

.PHONY: clean gbn

AlternatingBit: prog2_AlternatingBit.c
	mkdir -p out
	$(CC) $(CFLAGS) -o out/NetEmu prog2_AlternatingBit.c

gbn:
	mkdir -p out
	$(CC) $(CFLAGS) -o out/NetEmu_gbn -DGO_BACK_N prog2.c

clean:
	rm -rf out
