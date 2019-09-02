# Interprocess-Communication-Using-Named-Pipes
CPP, IPC, Named Pipes

# About
This repository is part of a coding challenge. There are 2 windows console applications developed using C++: The server and a client that communicate via Named pipes.


## Client
PipedClient represents the client application and contains every action that it can do on the server. It can read and write data on the server pipe. The pipe client opens a named pipe, sets the pipe handle to message-read mode, uses the WriteFile function to send a request to the server, and uses the ReadFile function to read the server's reply.

The client also presents the user with a customized menu having the following options => 
Option 1: Insert Data
Option 2: Read data from server
Option 0 : Close client

## Server
PipedServer represents the server application that is multithreaded. It has a main thread with a loop that creates a pipe instance and waits for a pipe client to connect. When a pipe client connects, the pipe server creates a thread to service that client and then continues to execute the loop in the main thread. The thread created to service each pipe instance reads requests from the pipe ,stores the data in a vector and writes replies to the pipe until the pipe client closes its handle.


## Data
As the name says, its the class responsible for holding the data that goes through the pipe. Currently has only the `name` attribute. It also has operators `<<` and `=` overloaded for making the cout's and variable attributions easier. The method toString() also makes it easier to use since the pipe functions that are implemented accept only std::strings as parameters currently. 
