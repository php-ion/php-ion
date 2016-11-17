FROM ubuntu:precise

MAINTAINER Ivan Shalagnov <ivan@shalganov.me>

ENV DEBIAN_FRONTEND noninteractive

# Setup debian
RUN apt-get update && apt-get install -y \
    bison \
    nano-tiny \
    vim-tiny \
    less \
    autoconf \
    automake \
    libtool \
    lcov \
    file \
    g++ \
    gcc \
    libc-dev \
    make \
    pkg-config \
    re2c \
    ca-certificates \
    curl \
    libedit2 \
    libsqlite3-0 \
    libxml2 \
    xz-utils \
    gdb \
    gdbserver \
    git-core \
    libcurl4-openssl-dev \
    libedit-dev \
    libsqlite3-dev \
    libssl-dev \
    libxml2-dev \
    libevent-dev \
    unzip         \
    tcpdump \
    aptitude \
    linux-generic-lts-trusty \
    --no-install-recommends

CMD touch /tmp/lock && tail -F /tmp/lock