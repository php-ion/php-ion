Segfault occurred
=================

[Run the tests](./testing.md#testing-php-ion) (required system packages: `automake`, `libtool`, `gcc`):

```
git clone https://github.com/php-ion/php-ion.git 
cd php-ion                                       
git submodule update --init --recursive          
composer install                                 
bin/ionizer.php --debug --system --clean-deps --clean --prepare --make --info --test --use-gdb
```

### If segfault occurred or tests failed:

1. Create an [issue](https://github.com/php-ion/php-ion/issues/new)
2. Copy output from tests into issue or [gist](https://gist.github.com/)

### Test OK but segfault still occurred:

1. Install PHP develop package in your system (`php-dev` or `php-debug` or `php-gdb` or all of them)
2. Install `gdb` package
3. Run your program using gdb:
   `gdb -ex "handle SIGHUP nostop" -ex "run" -ex "thread apply all bt" -ex "set pagination 0" -batch -return-child-result -silent --args PROGRAM [PROGRAM_ARGS ...]`
4. Copy backtrace after segfault into issue
