
#include "php.h"
#include "pion.h"

BEGIN_EXTERN_C()

int php_stream_get_fd(zval *zfd) {

    php_stream *stream;
    int fd = -1;
    if (ZEND_FETCH_RESOURCE_NO_RETURN(stream, php_stream *, &zfd, -1, NULL, php_file_le_stream())) {
        if (php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void *) &fd, REPORT_ERRORS) != FAILURE) {
            return fd;
        }
    }
    return -1;
}

END_EXTERN_C()