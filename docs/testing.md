Testing PHP-ION
=======

Test absilable if the extension installed [from sources](./install.md#build-from-source).

`$PHP_ION_DIR` - path to extension's sources (see [insallation](./install.md#build-from-source))


## Automatic way

```
$PHP_ION_DIR/bin/ionizer.php --debug --system --clean-deps --clean --prepare --make --info --test
```

## Manual way

Prepare tests
```
cd $PHP_ION_DIR
composer install
```

Testing installed extension
```
php vendor/bin/phpunit
```

**or** testing builded extension
```
php -dextension=$PHP_ION_DIR/src/modules/ion.so vendor/bin/phpunit
```

**or** testing extension by path
```
php -dextension=/path/to/ion.so vendor/bin/phpunit
```
