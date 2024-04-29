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
    received_byte=$(timeout "${timeout_duration}" nc "${target}" "${port}")
    if [ -z "${received_byte}" ]; then
        echo "Timeout reached. Waiting for next data..."
    else
        echo "Received byte data: ${received_byte}"
    fi
done
