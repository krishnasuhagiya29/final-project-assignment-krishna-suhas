#!/bin/bash

PORT=5000
HOST="0.0.0.0"
PIPE="/tmp/test_server_pipe"
rm -f "$PIPE"
mkfifo "$PIPE"

trap "rm -f $PIPE; exit" SIGINT SIGTERM

# Function to start the server
start_server() {
    while true; do
        for speed in 60 80; do
            echo -ne "$(printf '\\x%02x' $speed)" > "$PIPE"
            sleep 1  # Ensure there's a delay for readability and processing
        done
    done | nc -l -p $PORT < "$PIPE"
}

start_server &
wait $!
