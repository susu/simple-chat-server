simple-chat-server
==================

A very simple multithreaded chat server in C++11, using BSD sockets.
For educational purposes (a kata).


Clone this repo such way:
```
git clone --recursive <repo>
```
Build
=====

```
mkdir build
cd build
cmake ../
make
```

Run
===

```
cd build
src/run_chatserver
```

Connect
=======

```
telnet localhost 5050
```
