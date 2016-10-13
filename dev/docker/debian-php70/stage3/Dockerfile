FROM ion-dbg:stage2

MAINTAINER Ivan Shalagnov <ivan@shalganov.me>

ENV DEBIAN_FRONTEND noninteractive

EXPOSE 8017

RUN mkdir -p /data/ion-dbg
RUN mkdir -p /data/ion-dev


RUN php -r "readfile('https://getcomposer.org/installer');" | php
RUN mv composer.phar /usr/bin/composer

#RUN git clone https://github.com/php-ion/php-ion.git /data/ion-dev
#WORKDIR /data/ion-dev
#RUN git submodule update --init --recursive
#RUN composer install
#RUN bin/ionizer.php --prepare --make --info
#CMD bin/ionizer.php --dev --resync=/data/ion-orig:/data/ion-dev --make --gdb-server=127.0.0.1:8017
#CMD php -dextension=src/modiles/ion.so -a

#CMD gdbserver --debug --remote-debug 0.0.0.0:8017 php -m
CMD touch /tmp/lock && tail -F /tmp/lock