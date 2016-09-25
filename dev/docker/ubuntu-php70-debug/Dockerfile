FROM php:7.0-cli

MAINTAINER Ivan Shalagnov <ivan@shalganov.me>

ENV PHP_EXTRA_CONFIGURE_ARGS "--enable-debug"
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y gdb \
		gdbserver \
		git-core \
        libevent-dev

#ADD sync.sh /data

EXPOSE 2345

RUN mkdir -p /data/ion-dev
RUN mkdir -p /data/ion-orig

WORKDIR /root

RUN php -r "readfile('https://getcomposer.org/installer');" | php
RUN mv composer.phar /usr/bin/composer

RUN git clone https://github.com/php-ion/php-ion.git /data/ion-dev
WORKDIR /data/ion-dev
RUN git submodule update --init --recursive
RUN bin/ionizer.php --phpize --make --info
#CMD bin/ionizer.php --dev --resync=/data/ion-orig:/data/ion-dev --make --gdb-server=127.0.0.1:2345
CMD bash
