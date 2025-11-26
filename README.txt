vcalc - C++ single-threaded TCP server and client (port of previous C project)

Build:
    make

Run server:
    ./server -d users.txt -l server.log -p 33333

Run default client:
    ./client 127.0.0.1 33333 user1 password1

You can also use your previously provided client (client_float.c) — it is included in the archive as client_float.c.