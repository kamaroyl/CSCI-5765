# Lab 1: FTP Server
### Source Code Layout
The source code is found under the src directory and divided by purpose; Server code is found under server, Client code
 under client and common functions for sys calls and input scraping under common. Common contains all of the structs
 used between the server and client. 
 
### To Build
While in lab1 directory run:
 * `make server`
 * `make client`
 
 The both executable files will be found under the `out` directory. <br>

### To Run
After building the executables will be available. You can move the executibles out from under the `out` directory
To run the server, in the directory where the `server` executable is, run `./server` . 
The client executable cannot be run until the server is running. To start the client, in the executable directory run `./client ip` where `ip` is the ip where the server executable is running.

<b> E.G. </b> <br>
`out/server` <br>
`out/client 127.0.0.1` <br>

### Tests - Unit
`make unit` compiles some unit tests, found under the test directory. It compiles to `out/unitTest`

### Example pictures
Added example pictures under the `Data` directory demonstrating the server running on csegrid with 2 clients and 4 clients running locally (Note was also tested with clients on csegrid).

