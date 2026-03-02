FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libboost-all-dev \
    libssl-dev \
    wget \
    poppler-utils

WORKDIR /app

COPY . .

# Build the project
RUN mkdir build && cd build && cmake .. && make

EXPOSE 18080

CMD ["./build/server"]