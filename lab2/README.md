# Lab 2 - Alternating Bit and Go Back N

Built on top of J.F.Kurose's Emulator supplied with "Computer Networking A Top Down Approach". </b>

## Common
Used the checksum described in the text (using 16 bit words) for checksumming


## Alternating bit
* `make clean`
* `make` 
* `out/NetEmu`
<br>
Should work with the inputs `100, .1, .3, 10, 2` and better (i.e. fewer errors/drops and higher
mean time to arrival).<b>
Implemented stop and wait so many packets are lost, but the emulation seems to work as expected.
<br>
 
### Design:
Ended up having to cache the last known packet that was successfully responded to; otherwise
the mean free path to the input getting accepted was very long. 
<br><br>
InputB specifically needed to check the cache, check if the packet is valid and then check
if the sequence number is as expected.

## Go Back N
* `make clean`
* `make gbn`
* `out/NetEmu_gbn`
<br>
Should work with inputs `50, .2, .2, 10, 2`
due to the output of GBN being fairly verbose it is being written to a file gbn.log <br>

### Design:
Initial design used individual acks, but, with over a third of packets lost to corruption
or dropping, the queue built up too fast. So, instead, I switched to cumulative acks. This 
worked much better and could actually successfully deliver 20+ packets.

A similar queue to the alternating bit implementation is used, but with the addition
of some additional information, mainly time and state of the packets. 

When a message is received by A, it is transformed into a packet, which is sent and pushed
onto the queue. if A input receives an ack, it sets the state
of all packets in the queue up to the received seqnum on the ack to ackd (these are then 
removed on the next timeout).
On timeout, if the packet has been acked, and there are no
 packets before it that are pending, it is removed from the queue. The first non-ackd 
 packet has its time checked and diffed. If the diff is greater than zero, the timer is
 reset to that value. Otherwise, all packets from that packet to the Window Size (or queue
 length) are set to pending and resent. 
