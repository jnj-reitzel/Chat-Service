# README.md

## Description
The project implements a chat service. A server application hosts the chat room.
A client application connects to this chatroom via TCP/IP.
After the client application has successfully connected to the server, the user can send his name and start chatting.
The server receives all messages and broadcasts them to all connected users.

## Dependencies
The project depends on the C++ standard library and Boost.Asio.
A bash script `install-dependencies.sh` is provided.

## Build
The build can be done using GNU Make.
The source code should be cross-platform compilable.
The Makefile uses `g++` as a compiler.
Call `make` in the top directory to build the applications.

## Run
First run `./bin/server` to start the chat server.
Then run `./bin/client` to start a client application.
The connection is made over TCP/IP.
By default the server accepts and the clients connect to `localhost` port `8080`.
However, you can specify an **IP** address and **PORT** number as the first and second program arguments.

## Tested
The applications have been tested on Ubuntu 22.04.3 LTS with g++ 12.3.
