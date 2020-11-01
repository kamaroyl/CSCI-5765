# Lab 2 - Alternating Bit and Go Back N

Built on top of J.F.Kurose's Emulator supplied with "Computer Networking A Top Down Approach". </b>

## Common
Used the checksum described in the text (using 16 bit words) for checksumming


## Alternating bit
* `make clean`
* `make` 
* `out/NetEmu`
<b>
Should work with the inputs `100, .2, .2, 10` and better (i.e. fewer errors/drops and higher
mean time to arrival).<b>
Implemented stop and wait so many packets are lost, but the emulation seems to work as expected.
<b>
### Design:
Ended up having to cache the last known packet that was successfully responded to; otherwise
the mean free path to the input getting accepted was very long. 
<b><b>
InputB specifically needed to check the cache, check if the packet is valid and then check
if the sequence number is as expected.

## Go Back N
DOES NOT CURRENTLY WORK
* `make clean`
* `make gbn`
* `out/NetEmu_gbn`

### Design:
Needed to build a queue that kept the currently pending packets and the time at which they
were sent. 
On interrupt
1) Grab the timestamp of the head of the queue (assumed to be the start time of the timer)
2) Check from the beginning of the queue, and while the packet states are acked, remove them.
3) Grab timestamp from first entry in queue that isn't acked, diff it with the head of the queue.
4) If the diff is greater than