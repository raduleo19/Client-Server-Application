# Client-Server-Application

## Arhitecture
To start, I will talk about the tcp protocol. My implementation uses a thin client. 
So the tcp packet contains only the size and message to be displayed on the client. 
Rendering is done on the server. 

### Validation
I first validate the orders on the client, 
in order to provide real-time feedback and not to send messages to the 
server unnecessarily. Then I validate them once more on the server, to make 
sure that no malicious message is sent by an application other than the 
one I implemented. Having valid messages, the server analyzes the messages and 
sends to the client answers such as (subscribed, unsubscribe, alread_subscribed, not a subscriber). 

### Safe exiting
For the exit command, to send all the messages already on the way, 
I close the write part of the connection (with shutdown), 
which notifies the server that I have disconnected, 
and when the client receives the close notification(positioned on system stack 
after the rest of pending messages) from server I close the client.

### Message Delimitation Over Tcp
To delimit the messages on tcp, I used the size field, 
put for this purpose in the package. Thus, I created my TcpBroker class, 
which deals with the delimitation of these, reading non-blocking, 
until an entire packet is completed.

### Data Storage
For data storage, first of all I needed a map from sockets to ids.
Then I needed a map from sockets to addresses, to be able to display 
the port and address when I non-blocking receive the id, 
which can be done at any time, thus accepting slower devices.
Then I keep a chained list (in order to be able to delete in constant complexity) 
with the messages (the number of persistent offline followers and the actual message). T
hus, when a message has reached all the followers who were offline when it was published, 
we delete it from the list.
Each socket also has its own broker that receives (delimited) messages on that socket.
It sends blocking the messages (because they must be sent now, with no other way to trigger this action again).
Also each topic has a list (an unordered_map for fast operations and O (N) iteration) with each subscriber and its type (sf or not sf :)).
The last auxiliary structure is a map from id to subcriber. Each subscriber contains the socket (or -1 if it is not online), 
the number of topics with sf (if it does not exist we can delete it from memory to disconnect) 
and a vector to the elements from the global message list (kept as iterator for fast delete ).

### Sockets Misc
The sockets have been implemented so that they can be reused, and neagle's algorithm has been disabled.

### Coding Style
The coding style used is the one from google, 
except for trailing underscore on class members.
Also I preferred to use the pragma once(msvc specific) instead of guards.

### Memory Status
I tested with valgrind and I got no errors and no memory leaks.

### How to use:
#### Building
* Open a terminal/console/command prompt
    ````
    make build
    ````
#### Running
    ````
    ./subscriber <ID_Client> <IP_Server> <Port_Server>
    ./server <PORT>
    ````
#### Cleaning
    ````
    .make clean
    ````

## Resources used
### Disable Nagles Algorithm
https://stackoverflow.com/questions/17842406/how-would-one-disable-nagles-algorithm-in-linux
### Reuse socket port
https://lwn.net/Articles/542629/
### Multiplexing and fd_set
https://linux.die.net/man/3/fd_set
### Sockets Operations
https://www.geeksforgeeks.org/socket-programming-cc/
### Shutdown Descriptor
https://linux.die.net/man/3/shutdown
### Why I used unordered_set/unordered_map
https://stackoverflow.com/questions/25195233/time-complexity-of-iterating-through-a-c-unordered-map
### Why I double validate data
https://stackoverflow.com/questions/162159/javascript-client-side-vs-server-side-validation

### Author: Rica Radu-Leonard 2020
