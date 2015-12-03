FROM ubuntu:14.04

MAINTAINER Ivan Shalagnov <ivan@shalganov.me>

ENV HOME /root
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y supervisor \
		openssh-server vim-tiny \
		xfce4 xfce4-goodies \
		x11vnc xvfb \
		firefox \
		aufs-tools \
        automake \
        bison \
        btrfs-tools \
        build-essential \
        curl \
        git \
        libbz2-dev \
        libcurl4-openssl-dev \
        libmcrypt-dev \
        libxml2-dev \
        re2c \
        gdb  \
        libevent-dev \
        software-properties-common \
	&& apt-get autoclean \
	&& apt-get autoremove \
	&& rm -rf /var/lib/apt/lists/*

RUN echo oracle-java8-installer shared/accepted-oracle-license-v1-1 select true | debconf-set-selections
RUN add-apt-repository -y ppa:webupd8team/java
RUN apt-get update && apt-get install -y oracle-java8-installer \
    && rm -rf /var/cache/oracle-jdk8-installer

WORKDIR /root

COPY startup.sh /root/
COPY supervisord.conf /root/

EXPOSE 5900
EXPOSE 22

## Define commonly used JAVA_HOME variable
ENV JAVA_HOME /usr/lib/jvm/java-8-oracle
ENV PHP_DIR /root/php
ENV CFLAGS "$CFLAGS -Wall -g3 -ggdb -O0 -std=gnu99"

RUN wget -q 'https://download.jetbrains.com/cpp/clion-1.2.1.tar.gz'
RUN tar -xzf 'clion-1.2.1.tar.gz'

RUN git clone --depth=1 https://github.com/php/php-src.git /root/src/php

WORKDIR /root/src/php

## configure the build
RUN ./buildconf && ./configure \
    --prefix=$PHP_DIR \
    --with-config-file-path=$PHP_DIR \
    --with-config-file-scan-dir=$PHP_DIR/conf.d \
    --with-libdir=/lib/x86_64-linux-gnu \
    --includedir=/usr/include/x86_64-linux-gnu \
    --enable-bcmath \
    --with-bz2 \
    --enable-calendar \
    --with-curl \
    --enable-mbstring \
    --enable-mbregex \
    --with-mcrypt \
    --with-openssl \
    --enable-pcntl \
    --without-pear \
    --disable-pdo \
    --enable-sockets \
    --with-zlib \
    --enable-maintainer-zts \
    --enable-debug

## compile and install
RUN make && make install

ENV PATH $PATH:/root/php

WORKDIR /root/

RUN php -r "readfile('https://getcomposer.org/installer');" | php
RUN mv composer.phar /usr/bin/composer

ENTRYPOINT ["/root/startup.sh"]
