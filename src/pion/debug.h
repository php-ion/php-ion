
#ifndef ION_DEBUG_H
#define ION_DEBUG_H


#define PHPDBG(msg, ...)    \
    printf("%s: ", __func__); \
    printf(msg, ##__VA_ARGS__); \
    printf("\n");

#define ZVAL_DUMP_PP(zvar)  ZVAL_DUMP_P(*zvar)

#define ZVAL_DUMP_P(zvar)  \
    php_printf("DUMP: "); \
    php_var_dump(&(zvar), 1 TSRMLS_CC);

#define ZVAL_DUMPF(zvar, format, ...)  \
    php_printf(format, ##__VA_ARGS__); \
    php_var_dump(&zvar, 1 TSRMLS_CC);

#define IONF(msg, ...)

#endif //ION_DEBUG_H
