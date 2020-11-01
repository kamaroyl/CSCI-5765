#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifdef ENABLE_BIDIRECTIONAL
#define BIDIRECTIONAL 1
#else
#define BIDIRECTIONAL 0
#endif

#define WINDOW_SIZE 8
#define BUFFER_SIZE 50
#define A 0
#define B 1

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: 

   VERSION 1.1  J.F.Kurose
   Revised 1.2  IK Ra
   REVISED 1.3  B. Straub

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
};

/** Forward Declare Callable Methods **/
void init();
void tolayer5(int AorB,char datasent[20]);
void tolayer3(int AorB,struct pkt packet);
void startTimer(int AorB, float increment);
void stopTimer(int AorB);
float getTime();

/** Utility Methods **/
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
    return (char)(((input & 0x20000) >> 16) & 1) ;
}

int get16BitWordFrom2Chars(char input1, char input2) {
    int out = (input1 << 8) + input2;
    return out;
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

void printMessage(struct msg*messagePtr) {
    int i = 0;
    printf("MSG: { data: [");
    for(i; i < 19; ++i)
    {
        printf("%c,", messagePtr->data[i]);
    }
    printf("%c] }\n", messagePtr->data[19]);
}

struct msg packetToMessage(struct pkt* packet) {
    struct msg message;
    for(int i = 0; i < 20; ++i) {
        message.data[i] = packet->payload[i];
    }

    return message;
}

struct pkt createPacketFromMsg(struct msg* message, int sequenceNumber) {
    struct pkt packet;
    packet.seqnum = sequenceNumber;
    packet.acknum = 0;
    for(int i = 0; i < 20; i++) {
        packet.payload[i] = message->data[i];

    }
    packet.checksum = calculateChecksum(&packet);
    return packet;
}

void emptyData(char data[20]) {
    for (int i = 0; i < 20; i++) {
        data[i] = 0;
    }
}

void printPacket(struct pkt* packetPtr) {
    printf("PKT: { ");
    printf("sequence_number: %d, ", packetPtr->seqnum);
    printf("acknowledgement_number: %d, ", packetPtr->acknum);
    printf("check_sum: %d, ", packetPtr->checksum);
    printf("payload: [");
    for(int i = 0; i < 19; ++i) {
        printf("%c,", packetPtr->payload[i]);
    }
    printf("%c] }\n", packetPtr->payload[19]);
}



/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/** Student Struct Declarations **/
enum state {
    start,
    wait,
    ready
};

enum PacketState {
    new,
    pending,
    ackd
};

struct PacketHolder {
    struct pkt* packet;
    float time;
    enum PacketState state;
};

struct PacketHolderQueue {
    struct PacketHolder* packets[BUFFER_SIZE + 1];
    int capacity;
    int length;
};

/** Student Globals **/
static int Base;
static struct PacketHolderQueue packetQA;
static enum state AState;
static int CurrentSequenceNumberA;
static int CurrentSequenceNumberB;

static int TIMEOUT = 20;

static struct pkt* peek(struct PacketHolderQueue* packetQ) {
    if(packetQ->length == 0) return NULL;
    return packetQ->packets[0]->packet;
}

static struct pkt* pop(struct PacketHolderQueue* packetQ) {
    if(packetQ->length == 0) return NULL;
    struct PacketHolder* phPtr = packetQ->packets[0];
    struct pkt* result = phPtr->packet;
    for (int i = 1; i < packetQ->length; i++) {
        packetQ->packets[i - 1] = packetQ->packets[i];
    }
    packetQ[packetQ->length] = NULL;
    free(phPtr);
    packetQ->length--;
    return result;
}

static void setPacketState(struct PacketHolderQueue* packetQ, enum PacketState state, int seqNum) {
    if (packetQ->length == 0) {
        return;
    }
    for(int i = 0; i < packetQ->length; i++) {
        if (packetQ->packets[i]->packet->seqnum == seqNum) {
            packetQ->packets[i]->state = state;
            return;
        }
    }
}

static void find

static int push(struct PacketHolderQueue* packetQ, struct pkt* packet) {
    struct pkt* localPacket = (struct pkt*) malloc(sizeof(struct pkt));
    localPacket->acknum = packet->acknum;
    localPacket->seqnum = packet->seqnum;
    localPacket->checksum = packet->checksum;
    for(int i = 0; i < 20; ++i) {
        localPacket->payload[i] = packet->payload[i];
    }

    struct PacketHolder* packetHolder = (struct PacketHolder*) malloc(sizeof(struct PacketHolder));
    packetHolder->state = new;
    packetHolder->packet = localPacket;
    packetHolder->time = getTime();
    printf("PacketQ Length: %d\nPacketQ Capacity: %d\n", packetQ->length, packetQ->capacity);
    if (packetQ->length < packetQ->capacity) {
        packetQ->packets[packetQ->length] = packetHolder;
        packetQ->length++;
        printf("PacketQ Length is now: %d\n", packetQ->length);
        return 0;
    }
    return 1;
}

static char isQFull(struct PacketHolderQueue* packetQ) {
    return packetQ->length == packetQ->capacity;
}

static char isWindowFull(struct PacketHolderQueue* packetQ) {
    int count = 0;
    for(int i = 0; i < packetQ->length; i++) {
        if (packetQ->packets[i]->state == pending) count++;
        if (count == WINDOW_SIZE) return 1;
    }
    return 0;
}

static int getSeqNumA() {
    CurrentSequenceNumberA = (CurrentSequenceNumberA + 1);
    return CurrentSequenceNumberA;
}

static int getSeqNumB() {
    CurrentSequenceNumberB = (CurrentSequenceNumberB + 1) % WINDOW_SIZE;
    return CurrentSequenceNumberB;
}

static int getNackA() {
    return -1;
}

static int getNackB() {
    return -1;
}

float getTimeDiff(float storedTime, float currentTime) {
    return currentTime - storedTime;
}

/* called from layer 5, passed the data to be sent to B */
void outputA(struct msg message) {
    printf("Output A\n");
    printMessage(&message);

    /* If there are over 50 waiting packets exit*/
    if (isQFull(&packetQA)) {
        printf("Buffer overflow, exiting\n");
        exit(1);
    }
    /* Transform message to packet */
    struct pkt packetToSend = createPacketFromMsg(&message, getSeqNumA());
    push(&packetQA, &packetToSend);
    printf("Output A Packet Translated\n");
    printPacket(&packetToSend);
    if (!isWindowFull(&packetQA)) {
        setPacketState(&packetQA, pending, packetToSend.seqnum);
        tolayer3(A, packetToSend);
    }
}

void outputB(struct msg message) {
}

/* B send to A, called from layer 3, *
 * when a packet arrives for layer 4 */
void inputA(struct pkt packet) {
    printf("Input A\n");
    printPacket(&packet);

    /* Check if packet is valid */
    /* Drop if not valid */
    if(!validateChecksum(&packet))  {
        printf("Checksum was invalid\n");
        return;
    }

    /* wasn't sure how to indicate it was a nack so*/
    /* acknum = -1 since it's a 32 bit integer representing */
    /* a binary flag and it felt bad putting that info in */
    /* Sequence Number */
    if(packet.acknum == -1) {
        printf("Packet was a NACK, resend at next interrupt\n");
        return;
    }

    /* Transform packet to message */
    struct msg message = packetToMessage(&packet);
    tolayer5(A,message.data);
    stopTimer(A);
    AState = ready;
    getSeqNumA();
    struct pkt* packetToFree = pop(&packetQA);
    if (packetToFree != NULL) {
        free(packetToFree);
    }
}

/* called when A's timer goes off */
void timerInterruptA(void) {
    /* Go through the list of currently pending transactions */

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void initA(void) {
    Base = 0;
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void inputB(struct pkt packet) {
    printf("Input B\n");
    printPacket(&packet);
    struct pkt response;
    emptyData(response.payload);
    response.seqnum = 0;

    /* If packet has some error, send nack*/
    if (!validateChecksum(&packet)) {
        printf("Incoming packet was corrupted\n");
        response.seqnum = CurrentSequenceNumberB;
        response.acknum = -1;
    } else if (packet.seqNum % WINDOW_SIZE == CurrentSequenceNumberB){
        struct msg message;
        message = packetToMessage(&packet);
        printf("Translated packet to message\n");
        printMessage(&message);
        tolayer5(B, message.data);
        response.acknum = packet.seqnum;
        getSeqNumB();
    } else {
        printf("Unexpected sequence Number\n");
        response.seqnum = CurrentSequenceNumberB;
        response.acknum = -1
    }

    response.checksum = calculateChecksum(&response);
    tolayer3(B, response);
}

/* called when B's timer goes off */
void timerInterruptB(void) {
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void initB(void){
    CurrentSequenceNumberB = 1;
}
#ifdef GO_BACK_N

#else /** ALTERNATING BIT **/

/** Student Globals *//
static int Base;
static struct PacketQueue packetQA;
static struct msg* AMessageQ[BUFFER_SIZE];
static struct msg* BMessageQ[BUFFER_SIZE];
static enum state AState;
static int CurrentSequenceNumberA;
static int CurrentSequenceNumberB;

static int TIMEOUT = 20;

static struct pkt* peek(struct PacketQueue* packetQ) {
    if(packetQ->length == 0) return NULL;
    return packetQ->packets[0];
}

static struct pkt* pop(struct PacketQueue* packetQ) {
    if(packetQ->length == 0) return NULL;
    struct pkt* result = packetQ->packets[0];
    for (int i = 1; i < packetQ->length; i++) {
        packetQ->packets[i - 1] = packetQ->packets[i];
    }
    packetQ->length--;
    return result;
}

static int push(struct PacketQueue* packetQ, struct pkt* packet) {
    struct pkt* localPacket = (struct pkt*) malloc(sizeof(struct pkt));
    localPacket->acknum = packet->acknum;
    localPacket->seqnum = packet->seqnum;
    localPacket->checksum = packet->checksum;

    for(int i = 0; i < 20; ++i) {
        localPacket->payload[i] = packet->payload[i];
    }

    printf("PacketQ Length: %d\nPacketQ Capacity: %d\n", packetQ->length, packetQ->capacity);
    if (packetQ->length < packetQ->capacity) {
        packetQ->packets[packetQ->length] = localPacket;
        packetQ->length++;
        printf("PacketQ Length is now: %d\n", packetQ->length);
        return 0;
    }
    return 1;
}

static char isQFull(struct PacketQueue* packetQ) {
    return packetQ->length == packetQ->capacity;
}

static int getSeqNumA() {
    CurrentSequenceNumberA ^= 1;
    return CurrentSequenceNumberA;
}

static int getSeqNumB() {
    CurrentSequenceNumberB ^= 1;
    return CurrentSequenceNumberB;
}

static int getNackA() {
    return CurrentSequenceNumberA ^ 1;
}

static int getNackB() {
    return CurrentSequenceNumberB ^ 1;
}

/* called from layer 5, passed the data to be sent to B */
void outputA(struct msg message) {
    printf("Output A\n");
    printMessage(&message);
    /* Stop and wait; drop packet if waiting */
    if(AState == wait) {
        return;
    }
    /* Transform message to packet */
    struct pkt packetToSend = createPacketFromMsg(&message,getNackA());
    push(&packetQA, &packetToSend);
    printf("Output A Packet Translated\n");
    printPacket(&packetToSend);
    tolayer3(A, packetToSend);
    AState = wait;
    startTimer(A, TIMEOUT);
}

void outputB(struct msg message) {
    printf("Output B\n");
    printMessage(&message);
}

/* B send to A, called from layer 3, *
 * when a packet arrives for layer 4 */
void inputA(struct pkt packet) {
    printf("Input A\n");
    printPacket(&packet);

    /* Check if packet is valid */
    /* Drop if not valid */
    if(!validateChecksum(&packet))  {
        printf("Checksum was invalid\n");
        return;
    }

    /* wasn't sure how to indicate it was a nack so*/
    /* acknum = -1 since it's a 32 bit integer representing */
    /* a binary flag and it felt bad putting that info in */
    /* Sequence Number */
    if(packet.acknum == -1) {
        stopTimer(A);
        printf("Packet was a NACK, resend\n");
        startTimer(A, TIMEOUT);
        struct pkt* packetToResend = peek(&packetQA);
        printPacket(packetToResend);
        if (packetToResend != NULL){
            tolayer3(A, *packetToResend);
            startTimer(A, TIMEOUT);
        } else {
            printf("Packet stored in Q was NULL");
        }
        return;
    }

    /* Transform packet to message */
    struct msg message = packetToMessage(&packet);
    tolayer5(A,message.data);
    stopTimer(A);
    AState = ready;
    getSeqNumA();
    struct pkt* packetToFree = pop(&packetQA);
    if (packetToFree != NULL) {
        free(packetToFree);
    }
}

/* called when A's timer goes off */
void timerInterruptA(void) {
    printf("Interrupt Timer A Fired\n");
    /* There has been a timeout, resend packet */
    struct pkt* packetToResend = peek(&packetQA);
    printf("A Interrupted packet resend: \n");
    printPacket(packetToResend);
    if (packetToResend != NULL){
        tolayer3(A, *packetToResend);
        startTimer(A, TIMEOUT);
    } else {
        printf("Packet stored in Q was NULL");
    }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void initA(void) {
    AState = start;
    CurrentSequenceNumberA = 0;
    packetQA.capacity = BUFFER_SIZE;
}


/* Note that with simplex transfer from a-to-B, there is no outputB() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void inputB(struct pkt packet) {
    printf("Input B\n");
    printPacket(&packet);
    struct pkt response;
    emptyData(response.payload);
    response.seqnum = 0;

    /* If packet has some error, send nack*/
    if (!validateChecksum(&packet)) {
        printf("Incoming packet was corrupted\n");
        response.acknum = -1;
    } else if (CurrentSequenceNumberB == packet.seqnum) {
        struct msg message;
        message = packetToMessage(&packet);
        printf("Translated packet to message\n");
        printMessage(&message);
        tolayer5(B, message.data);
        response.acknum = getSeqNumB();
    } else {
        printf("Unexpected sequence Number\n");

        response.acknum = getNackB();
    }

    response.checksum = calculateChecksum(&response);
    tolayer3(B, response);
}

/* called when B's timer goes off */
void timerInterruptB(void) {
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void initB(void){
    CurrentSequenceNumberB = 1;
}
#endif
/***************** END STUDENTS ROUTINES ******************/

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the transmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOULD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you shouldn't have
to, and you definitely should not have to modify
******************************************************************/

struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
 };
//Global event list
static struct event *evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1

void insertevent(struct event* p);
void printevlist();
void generate_next_arrival();

int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */ 
int nsimmax = 10;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob = 0.0;            /* probability that a packet is dropped  */
float corruptprob = 0.0;         /* probability that one bit is packet is flipped */
float lambda = 1000;              /* arrival rate of messages from layer 5 */
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

int main(void) {
    struct event *eventptr;
    struct msg  msg2give;
    struct pkt  pkt2give;
   
    int i,j;
    char c;
  
    init();
    initA();
    initB();
    while (1) {
        eventptr = evlist; /* get next event to simulate */
        if (eventptr==NULL) {
            goto terminate;
        }
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL) {
            evlist->prev = NULL;
        }
        if (TRACE >= 2) {
            printf("\nEVENT time: %f,",eventptr->evtime);
            printf("  type: %d",eventptr->evtype);
            if (eventptr->evtype == 0) {
                printf(", timerinterrupt  ");
            } else if (eventptr->evtype == 1) {
                printf(", fromlayer5 ");
            } else {
                printf(", fromlayer3 ");
            }
            printf(" entity: %d\n",eventptr->eventity);
        }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim==nsimmax) {
            break;                        /* all done with simulation */
        }
        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i = 0; i < 20; i++) {
                msg2give.data[i] = 97 + j;
            }
            if (TRACE > 2) {
                printf("          MAINLOOP: data given to student: ");
                for (i = 0; i < 20; i++) {
                    printf("%c", msg2give.data[i]);
                }
                printf("\n");
            }
            nsim++;
            if (eventptr->eventity == A) {
                outputA(msg2give);
            } else {
                outputB(msg2give);
            }
        } else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i = 0; i < 20; i++) {
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            }
            if (eventptr->eventity == A) {     /* deliver packet by calling */
                inputA(pkt2give);            /* appropriate entity */
            } else {
                inputB(pkt2give);
            }
	        free(eventptr->pktptr);          /* free the memory for packet */
        } else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A) {
                timerInterruptA();
            } else {
                timerInterruptB();
            }
        } else  {
            printf("INTERNAL PANIC: unknown event type \n");
            break;
        }
        free(eventptr);
    }

terminate:
   printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
   return 0;
}



void init() {                       /* initialize the simulator */
    void generate_next_arrival();

    int i;
    float sum, avg;
    float jimsrand();
    int tmpInt = 0;
  
    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
#ifdef GO_BACK_N
    printf("ALGORITHM: GO BACK N\n");
#else
    printf("ALGORITHM: ALTERNATING BIT\n");
#endif
    printf("Enter the number of messages to simulate: ");
    scanf("%d",&nsimmax);
    printf("Enter packet loss probability, range [0.0, 1.0]: ");
    scanf("%f",&lossprob);
    printf("Enter packet corruption probability, range [0.0, 1.0]: ");
    scanf("%f",&corruptprob);
    printf("Enter average time between messages from sender's layer5 1000 is good: ");
    scanf("%f",&lambda);
    printf("Enter TRACE [1, 2, 3]: ");
    scanf("%d",&TRACE);

    srand(9999);              /* init random number generator */
    sum = 0.0;                /* test random number generator for students */
    for (i = 0; i < 1000; i++) {
        sum += jimsrand();    /* jimsrand() should be uniform in [0,1] */
    }
    avg = sum/1000.0;
    if (avg < 0.25 || avg > 0.75) {
        printf("It is likely that random number generation on your machine\n" );
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(-1);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;                    /* initialize time to 0.0 */
    generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand() {
    double mmm = INT_MAX;      /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
    float x;                   /* individual students may need to change mmm */
    x = rand()/mmm;            /* x should be uniform in [0,1] */
    return(x);
}  

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/
 
void generate_next_arrival() {
    double x, log(), ceil();
    struct event *evptr;
    /* char *malloc(); */
    float ttime;
    int tempint;

    if (TRACE > 2) {
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");
    }
    x = lambda * jimsrand() * 2;  /* x is uniform on [0,2*lambda] */
    /* having mean of lambda        */
    evptr = (struct event *) malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;

    if (BIDIRECTIONAL && (jimsrand() > 0.5)) {
        evptr->eventity = B;
    } else {
        evptr->eventity = A;
    }

    insertevent(evptr);
} 


void insertevent(struct event* p) {
    struct event* q;
    struct event* qOld;

    if (TRACE>2) {
        printf("            INSERTEVENT: time is %lf\n",time);
        printf("            INSERTEVENT: future time will be %lf\n",p->evtime);
    }
    q = evlist;     /* q points to header of list in which p struct inserted */
    if (q == NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
    } else {
        for (qOld = q; q != NULL && p->evtime > q->evtime; q = q->next) {
            qOld = q;
        }
        if (q == NULL) {   /* end of list */
             qOld->next = p;
             p->prev = qOld;
             p->next = NULL;
        } else if (q == evlist) { /* front of list */
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        } else {     /* middle of list */
            p->next = q;
            p->prev = q->prev;
             q->prev->next = p;
             q->prev = p;
        }
    }
}

void printevlist() {
    struct event *q;
    printf("--------------\nEvent List Follows:\n");
    for(q = evlist; q != NULL; q = q->next) {
        printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
    printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stopTimer(int AorB) { /* A or B is trying to stop timer */
    struct event* q;
    struct event* qOld;
    if (TRACE > 2) {
        printf("          STOP TIMER: stopping timer at %f\n", time);
    }
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q=evlist; q!=NULL ; q = q->next) {
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL) {
                evlist = NULL;         /* remove first and only event on list */
            } else if (q->next == NULL) {/* end of list - there is one in front */
                q->prev->next = NULL;
            } else if (q == evlist) { /* front of list - there must be event after */
                q->next->prev = NULL;
                evlist = q->next;
            } else {     /* middle of list */
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    }
        printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


void startTimer(int AorB, float increment) { /* A or B is trying to stop timer */
    struct event *q;
    struct event *evptr;
    /* char *malloc(); */
    if (TRACE>2) {
        printf("          START TIMER: starting timer at %f\n",time);
    }
    /* be nice: check to see if timer is already started, if so, then  warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q=evlist; q!=NULL ; q = q->next) {
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }
    }
    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
} 


/************************** TOLAYER3 ***************/
void tolayer3(int AorB, struct pkt packet) { /* A or B is trying to stop timer */
    struct pkt* mypktptr;
    struct event* evptr;
    struct event* q;
    /* char *malloc(); */
    float lastime, x, jimsrand();
    int i;

    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob)  {
        nlost++;
        if (TRACE>0) {
            printf("          TOLAYER3: packet being lost\n");
        }
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */

    mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i = 0; i < 20; i++) {
        mypktptr->payload[i] = packet.payload[i];
    }
    if (TRACE>2)  {
        printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
               mypktptr->acknum,  mypktptr->checksum);
        for (i=0; i<20; i++) {
            printf("%c", mypktptr->payload[i]);
        }
        printf("\n");
   }

    /* create future event for arrival of packet at the other side */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtype = FROM_LAYER3;   /* packet will pop out from layer3 */
   evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
   evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
   /* finally, compute the arrival time of packet at the other end.
      medium can not reorder, so make sure packet arrives between 1 and 10
      time units after the latest arrival time of packets
      currently in the medium on their way to the destination */
   lastime = time;
   /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q=evlist; q!=NULL ; q = q->next) {
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity)) {
            lastime = q->evtime;
        }
    }
    evptr->evtime = lastime + 1 + 9 * jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob) {
        ncorrupt++;
        if ((x = jimsrand()) < .75) {
            mypktptr->payload[0] = 'Z';   /* corrupt payload */
        } else if (x < .875) {
            mypktptr->seqnum = 999999;
        } else {
            mypktptr->acknum = 999999;
        }

        if (TRACE>0) {
            printf("          TOLAYER3: packet being corrupted\n");
        }
    }  

    if (TRACE>2) {
        printf("          TOLAYER3: scheduling arrival on other side\n");
    }
    insertevent(evptr);
} 

void tolayer5(int AorB,char dataSent[20]) {
  int i;  
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i = 0; i < 20; i++) {
         printf("%c", dataSent[i]);
     }
     printf("\n");
   }
}

float getTime() {
    return time;
}
