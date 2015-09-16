
#ifndef PION_DEBUG_H
#define PION_DEBUG_H

//#include "ext/standard/php_var.h"

#define PHPDBG(msg, ...)    \
    fprintf(stderr, "%s: ", __func__); \
    fprintf(stderr,msg, ##__VA_ARGS__); \
    fprintf(stderr,"\n");

#define ZVAL_DUMP_PP(zvar)  ZVAL_DUMP_P(*zvar)

#define ZVAL_DUMP_P(zvar)  \
    php_printf("DUMP: "); \
    php_var_dump(&(zvar), 1 TSRMLS_CC);

#define ZVAL_DUMPF(zvar, format, ...)  \
    php_printf(format, ##__VA_ARGS__); \
    php_var_dump(&zvar, 1 TSRMLS_CC);

#define IONF(msg, ...)

#endif //PION_DEBUG_H
