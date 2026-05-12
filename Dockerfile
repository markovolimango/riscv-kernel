FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    cmake \
    tar \
    rsync \
    ssh \
    binutils-riscv64-linux-gnu \
    gcc-riscv64-linux-gnu \
    g++-riscv64-linux-gnu \
    gdb-multiarch \
    qemu-system-misc \
    && apt-get clean

WORKDIR /kernel