## battleships-socket

server runs as a daemon process w/ logging to syslog.

The server is capable of running multiple game sessions with many clients at once

client provides server auto-discovery feature using multicast address (default 224.0.0.200) so you don't need to spcify the IP address nor the port when connecting to the server, it works only in the local subnetwork

server by default binds to port 1111 (configurable)

partially based on the code by github.com/rougeth/


## How to play
To install this, simply run `make` command

run the server: `./server`

run the client(s): `./client`
