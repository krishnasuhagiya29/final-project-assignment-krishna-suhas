#!/bin/bash

# Client script to connect to the server and continuously receive byte data with a timeout

pushd "$(dirname "$0")"
target=localhost
port=9000
timeout_duration=10

echo "Connecting to server ${target} on port ${port}"
echo "Timeout duration set to ${timeout_duration} seconds"

# Connect to server and continuously receive byte data with a timeout
while true; do
    # Start a background process to read data from the server
    { sleep "${timeout_duration}"; echo "Timeout reached. Waiting for next data..."; } & timeout_pid=$!
    received_byte=$(nc "${target}" "${port}")
    
    # If data is received before the timeout, kill the timeout process
    kill "${timeout_pid}" > /dev/null 2>&1

    # Check if any byte data is received
    if [ -z "${received_byte}" ]; then
        echo "No data received. Waiting for next data..."
    else
        echo "Received byte data: ${received_byte}"
    fi
done
