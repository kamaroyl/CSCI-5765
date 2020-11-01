#include <stdio.h>
#include <assert.h>
struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

void printBinaryRep(int input) {
    for(int i = 31; i > 0; --i) {
        printf("%d", (input >> i) & 1);
    }
    printf("%d\n", (input) & 1);
}

int getTop16Bits(int input) {
    return (input >> 16) & 0xFFFF;
}

int getBottom16Bits(int input) {
    return (input & 0xFFFF);
}

char check17thBit(int input) {
    return (char)(((input & 0x10000) >> 16) & 1) ;
}

/* Take in a 32 bit signed int and sums it's two 16 bit halves. */
/* If there is a carry, bring the bit to the front */
int get1sCompSumOf16BitPartitioning(int input1, int input2) {
    int out = input1 + input2;
    return check17thBit(out) ? (out + 1) & 0xFFFF : out;
}

int get1sCompSumOfTwo16BitNumbersStoredIn32Bit(int input1, int input2) {
    int out = input1 + input2;
    return check17thBit(out) ? (out + 1) & 0xFFFF : out;
}

int get16BitWordFrom2Chars(char input1, char input2) {
    int out = (input1 << 8) + input2;
    return out;
}

int calculateChecksum(struct pkt* packet) {
    int sum = 0;
    sum = get1sCompSumOfTwo16BitNumbersStoredIn32Bit(getTop16Bits(packet->seqnum), getBottom16Bits(packet->seqnum));
    sum = get1sCompSumOfTwo16BitNumbersStoredIn32Bit(sum, getTop16Bits(packet->acknum));
    sum = get1sCompSumOfTwo16BitNumbersStoredIn32Bit(sum, getBottom16Bits(packet->acknum));
    for(int i = 0; i < 20; i += 2) {
        sum = get1sCompSumOfTwo16BitNumbersStoredIn32Bit(
                sum,
                get16BitWordFrom2Chars(
                        packet->payload[i],
                        packet->payload[i + 1]));
    }
    sum ^= 0xFFFF;
    return sum;
}

char validateChecksum(struct pkt* packet) {
    int sum = 0;
    sum = get1sCompSumOfTwo16BitNumbersStoredIn32Bit(getTop16Bits(packet->seqnum), getBottom16Bits(packet->seqnum));
    sum = get1sCompSumOfTwo16BitNumbersStoredIn32Bit(sum, getTop16Bits(packet->acknum));
    sum = get1sCompSumOfTwo16BitNumbersStoredIn32Bit(sum, getBottom16Bits(packet->acknum));
    for(int i = 0; i < 20; i += 2) {
        sum = get1sCompSumOfTwo16BitNumbersStoredIn32Bit(
                sum,
                get16BitWordFrom2Chars(
                        packet->payload[i],
                        packet->payload[i + 1]));
    }
    sum = get1sCompSumOfTwo16BitNumbersStoredIn32Bit(sum, getBottom16Bits(packet->checksum));

    return sum == 0xFFFF? 1: 0;
}

void test_getTop16Bits() {
    int input = -2130425631;
    assert(getTop16Bits(input) == 33028);
    printf("test_getTop16Bits Passed\n");
}

void test_getBottom16Bits() {
   int input = -2130425631;
   assert(getBottom16Bits(input) == 18657);
   printf("test_getBottom16Bits Passed\n");
}

void test_check17thBit() {
    int input_true = 0xfffff;
    int input_false = 0xffff;
    assert(check17thBit(input_true));
    assert(!check17thBit(input_false));
    printf("test_check17thBit Passed\n");
}

void test_get1sCompSumOf16BitPartitioning() {
    int input = 1717589333;
    assert(get1sCompSumOf16BitPartitioning(getTop16Bits(input), getBottom16Bits(input)) == 48053);
    //Test that the partition works with the rolling bit
    input = -1145729268;
    assert(get1sCompSumOf16BitPartitioning(getTop16Bits(input), getBottom16Bits(input)) == 19138);
    printf("test_get1sCompSumOf16BitPartitioning Passed\n");
}

void initPayload(char* input) {
    for(char i = 0; i < 20; ++i) {
        input[i] = 'A';
    }
}

void test_calculateChecksum() {
    struct pkt packet;
    char* handle = (char*) &(packet.payload);
    packet.seqnum = 123456; //57921
    packet.acknum = 87654321;
    initPayload(handle);
    packet.checksum = calculateChecksum(&packet);
    char isChecksumValid = validateChecksum(&packet);
    assert(isChecksumValid);
    printf("test_calculateChecksum Passed\n");
}

void test_calculateChecksum2() {
    struct pkt packet;
    char* handle = (char*) &(packet.payload);
    packet.seqnum = 0;
    packet.acknum = -1;
    initPayload(handle);
    packet.checksum = calculateChecksum(&packet);
    char isChecksumValid = validateChecksum(&packet);
    assert(isChecksumValid);
    printf("test_calculateChecksum2 Passed\n");
}

int main(void) {
    test_getTop16Bits();
    test_getBottom16Bits();
    test_check17thBit();
    test_get1sCompSumOf16BitPartitioning();
    test_calculateChecksum();
    test_calculateChecksum2();
    return 0;
}
