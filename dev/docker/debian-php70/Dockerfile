FROM debian:jessie

MAINTAINER Ivan Shalagnov <ivan@shalganov.me>

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
    autoconf \
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
    --no-install-recommends && rm -r /var/lib/apt/lists/*

# Build php

ENV PHP_INI_DIR /usr/local/etc/php
RUN mkdir -p $PHP_INI_DIR/conf.d
ENV PHP_RELEASE php-7.0.11

WORKDIR /usr/src


#RUN curl -fSL "https://github.com/php/php-src/archive/$PHP_RELEASE.zip" -o php-src.zip
ADD php.zip
RUN unzip php.zip

WORKDIR "/usr/src/php-src-$PHP_RELEASE"

RUN ./configure \
        \
		--with-config-file-path="$PHP_INI_DIR" \
        --with-config-file-scan-dir="$PHP_INI_DIR/conf.d" \
        \
        --disable-cgi \
        \
        --with-curl \
        --with-libedit \
        --with-openssl \
        --with-zlib \
        \
        --enable-debug \
        --enable-pcntl \
RUN make -j"$(nproc)"
RUN make install
RUN make clean
RUN php -m
