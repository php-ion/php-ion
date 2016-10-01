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

## /php/php-src/archive/PHP-7.0.11.zip
#ENV PHP_EXTRA_CONFIGURE_ARGS "--enable-debug --enable-pcntl"


#EXPOSE 8017
#
#RUN mkdir -p /data/ion-dev
#RUN mkdir -p /data/ion-orig
#
#WORKDIR /root
#
#RUN php -r "readfile('https://getcomposer.org/installer');" | php
#RUN mv composer.phar /usr/bin/composer
#
#RUN git clone https://github.com/php-ion/php-ion.git /data/ion-dev
#WORKDIR /data/ion-dev
#RUN git submodule update --init --recursive
#RUN composer install
#RUN bin/ionizer.php --phpize --make --info
##CMD bin/ionizer.php --dev --resync=/data/ion-orig:/data/ion-dev --make --gdb-server=127.0.0.1:8017
##CMD php -dextension=src/modiles/ion.so -a
#
#RUN touch /tmp/lock
#CMD tail -F /tmp/lock