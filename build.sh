#!/bin/bash

IMAGE_NAME="innoverse-patcher"
DOCKERFILE="Dockerfile"

if ! command -v docker &> /dev/null; then
    echo "Docker is not installed. Please install Docker and try again."
    exit 1
fi

echo "Building Docker image: $IMAGE_NAME..."
docker build . -t "$IMAGE_NAME"

if [ $? -ne 0 ]; then
    echo "Failed to build Docker image."
    exit 1
fi

echo "Compiling inside the Docker container..."
docker run -it --rm -v .:/app -w /app "$IMAGE_NAME"

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
else
    echo "Compilation failed."
    exit 1
fi
