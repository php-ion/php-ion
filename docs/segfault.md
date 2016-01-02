Segfault occurred
=================

At first run tests:

```
cd /tmp
git clone https://github.com/php-ion/php-ion.git
cd php-ion
curl -sS https://getcomposer.org/installer | php
./composer.phar install
bin/ionizer.php -pmiIct --use-gdb
```

If segfault occurred or tests failed:

1. Create [issue](https://github.com/php-ion/php-ion/issues/new)
2. Copy all tests' output into issue

Test OK and segfault still occurred:

1. Install PHP develop package in your system
2. Install `gdb` package
3. Run your program using gdb:
   `gdb -ex "handle SIGHUP nostop" -ex "run" -ex "thread apply all bt" -ex "set pagination 0" -batch -return-child-result -silent --args PROGRAM [PROGRAM_ARGS ...]`
4. Copy backtrace after segfault into issue