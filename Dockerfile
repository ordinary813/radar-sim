FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libsfml-dev \
    build-essential \
    cmake \
    gdb \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /app

WORKDIR /app

COPY . /app

RUN mkdir -p build
WORKDIR /app/build
RUN cmake ..
RUN make

WORKDIR /app

CMD ["./build/radar_sim"]
