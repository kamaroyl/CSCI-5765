CC = gcc
CFLAGS = -I -g 

.PHONY: clean gbn

AlternatingBit: prog2_AlternatingBit.c
	mkdir -p out
	$(CC) $(CFLAGS) -o out/NetEmu prog2_AlternatingBit.c

gbn: prog2_GoBackN.c
	mkdir -p out
	$(CC) $(CFLAGS) -o out/NetEmu_gbn -DGO_BACK_N prog2_GoBackN.c

clean:
	rm -rf out
