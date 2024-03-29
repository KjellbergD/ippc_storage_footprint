#!/bin/bash

# Check if the necessary protobuf files exist
if [ ! -f pipeline_config.pb-c.c ] || [ ! -f pipeline_config.pb-c.h ]; then
    echo "Error: Protocol Buffers files not found."
    exit 1
fi

# Compile the C program
gcc -g -o main main.c pipeline_config.pb-c.c cJSON.c -lprotobuf-c -lz

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Executing the program..."
    # Run the compiled program
    ./main
else
    echo "Compilation failed."
fi
