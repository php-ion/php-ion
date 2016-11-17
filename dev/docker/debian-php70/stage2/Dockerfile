FROM ion-dbg:stage1

MAINTAINER Ivan Shalagnov <ivan@shalganov.me>

ENV DEBIAN_FRONTEND noninteractive

# Build php
ENV PHP_INI_DIR /usr/local/etc/php
RUN mkdir -p $PHP_INI_DIR/conf.d
#ENV PHP_RELEASE php-7.0.11
ENV PHP_RELEASE php-7.1.0RC6

#RUN curl -fSL "https://github.com/php/php-src/archive/$PHP_RELEASE.zip" -o php-src.zip
ADD php.zip /usr/src/php-src.zip
#RUN unzip -q php-src.zip

ENV CFLAGS "-Wall -g3 -ggdb -O0"

RUN cd /usr/src \
    && unzip -q php-src.zip \
    && cd "/usr/src/php-src-$PHP_RELEASE" \
    && ./buildconf --force \
    && ./configure \
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
        --enable-pcntl  \
        --enable-maintainer-zts \
         \
     && make -j"$(nproc)" \
     && make install  \
     && make clean

RUN php -m