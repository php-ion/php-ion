#ifndef PION_CALLBACK_H
#define PION_CALLBACK_H

#include <php.h>

/* PHP callback */
typedef struct _pionCb {
    zend_fcall_info *fci;
    zend_fcall_info_cache *fcc;
#ifdef ZTS
    void ***thread_ctx;
#endif
} pionCb;

/* Create native callback */
pionCb * pionCbCreate(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr TSRMLS_DC);
pionCb * pionCbCreateFromZval(zval *zCb TSRMLS_DC);
void   pionCbFree(pionCb *cb);



/* Call callbacks */
int pionCbVoidCall(pionCb *cb, int num, zval ***args);
int pionCbVoidWithoutArgs(pionCb * cb);
int pionCbVoidWith1Arg(pionCb * cb, zval* arg1);
int pionCbVoidWith2Args(pionCb *cb, zval *arg1, zval *arg2);
int pionCbVoidWith3Args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3);
int pionCbVoidWith4Args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4);

int   pionCallConstructor(zend_class_entry *class_name, zval *object, int num_args, zval ***args);
/* Create an object */
zval* pionNewObject(zend_class_entry *ce, int num_args, zval ***args);
zval* pionNewObjectWithoutArgs(zend_class_entry *ce);
zval* pionNewObjectWith1Arg(zend_class_entry *ce, zval *arg1);
zval* pionNewObjectWith2Args(zend_class_entry *ce, zval *arg1, zval *arg2);
zval* pionNewObjectWith3Args(zend_class_entry *ce, zval *arg1, zval *arg2, zval *arg3);

zval* pionInitException(zend_class_entry *cls, char *message, int code);

/* Call PHP named function */
zval* pionCallFunction(const char *function_name, int num_args, zval **args);
zval* pionCallFunctionWithoutArgs(const char *function_name, zval *arg1);
zval* pionCallFunctionWith1Arg(const char *function_name, zval *arg1);
zval* pionCallFunctionWith2Args(const char *function_name, zval *arg1, zval *arg2);
zval* pionCallFunctionWith3Args(const char *function_name, zval *arg1, zval *arg2, zval *arg3);

#endif //PION_CALLBACK_H
