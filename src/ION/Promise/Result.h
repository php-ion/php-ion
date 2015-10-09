#ifndef ION_PROMISE_RESULT_H
#define ION_PROMISE_RESULT_H

#include "../../pion.h"

BEGIN_EXTERN_C();

typedef struct _ion_promise_result {
    zend_object        std;
    zval             * data;
#ifdef ZTS
    void ***thread_ctx;
#endif
} ion_promise_result;

zend_class_entry * ion_get_class(ION_Promise_Result);

END_EXTERN_C();

#endif //ION_PROMISE_RESULT_H
