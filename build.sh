#!/bin/bash

DOCKERFILE="Dockerfile"

if ! command -v docker &> /dev/null; then
    echo "Docker is not installed. Please install Docker and try again." # Need to download Docker or open Docker.
    exit 1
fi

echo "Building Docker image: innoverse-patcher.."
docker build . -t innoverse-patcher

if [ $? -ne 0 ]; then
    echo "Failed to build Docker image."
    exit 1
fi

echo "Compiling inside the Docker container..."
docker run -it --rm -v .:/app innoverse-patcher

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
else
    echo "Compilation failed."
    exit 1
fi
