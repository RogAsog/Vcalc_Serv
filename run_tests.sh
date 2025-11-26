#!/bin/bash
set -e
echo "Building..."
make
echo "Starting server in background..."
./server -d users.txt -l server.log -p 33333 &
SERVER_PID=$!
sleep 1
echo "Running client..."
./client 127.0.0.1 33333 user1 password1
echo "Killing server..."
kill $SERVER_PID || true
echo "Done."
