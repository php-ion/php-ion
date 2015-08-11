#include <php.h>
#include <event.h>

#include "ION.h"

DEFINE_CLASS(ION);
IONBase *ionBase;
void ion_reinit(long flags) {
    IONF("Reinit event loop: %d. Cleanup exec events...", (int)flags);
    //zend_hash_clean(ION(execs));

    if(!(flags & PRESERVE_TIMERS)) {
        IONF("Cleanup timer events...");
        zend_hash_clean(ION(timers));
    }

    if(!(flags & PRESERVE_SIGNALS)) {
        IONF("Cleanup signal events...");
        zend_hash_clean(ION(signals));
    }

    if(event_reinit(ION(base)) == FAILURE) {
        php_error(E_NOTICE, "Some events could not be re-added");
    }
}

/** public function ION::reinit(int $flags = 0) : self */
CLASS_METHOD(ION, reinit) {
    long flags = 0;
    PARSE_ARGS("|l", &flags);

    ion_reinit(flags);
}
METHOD_ARGS_BEGIN(ION, reinit, 1)
    METHOD_ARG_TYPE(flags, IS_LONG, 0, 0)
METHOD_ARGS_END()

/** public function ION::dispatch(int $flags = 0) : self */
CLASS_METHOD(ION, dispatch) {
    long flags = 0;
    int ret;

    PARSE_ARGS("|ld", &flags);

    ret = event_base_loop(ION(base), (int)flags);

    if(ret == -1) {
        ThrowRuntime("Dispatching runtime error", 1);
    }

    if(ret) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

METHOD_ARGS_BEGIN(ION, dispatch, 1)
    METHOD_ARG_TYPE(flags, IS_LONG, 0, 0)
METHOD_ARGS_END()

/** public function ION::stop(double $timeout = -1) : self */
CLASS_METHOD(ION, stop) {
    double timeout = 0.0;
    struct timeval time;
    PARSE_ARGS("|d", &timeout);

    if(timeout > 0) {
        time.tv_usec = (int)((int)(timeout*1000000) % 1000000);
        time.tv_sec = (int)timeout;
        event_base_loopexit(ION(base), &time);
    } else {
        event_base_loopbreak(ION(base));
    }
}

METHOD_ARGS_BEGIN(ION, stop, 1)
    METHOD_ARG_TYPE(timeout, IS_DOUBLE, 0, 0)
METHOD_ARGS_END()

CLASS_METHODS_START(ION)
    METHOD(ION, reinit,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, dispatch, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, stop,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
//    METHOD(ION_Deferred, then, ZEND_ACC_PUBLIC)
//    METHOD(ION_Deferred, reject, ZEND_ACC_PUBLIC)
//    METHOD(ION_Deferred, resolve, ZEND_ACC_PUBLIC)
//    METHOD(ION_Deferred, error, ZEND_ACC_PUBLIC)
//    METHOD(ION_Deferred, timeout, ZEND_ACC_PUBLIC)
//    METHOD(ION_Deferred, getFlags, ZEND_ACC_PUBLIC)
//    METHOD(ION_Deferred, __destruct, ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION) {
    PION_REGISTER_PLAIN_CLASS(ION, "ION");
    PION_CLASS_CONST_STRING(ION, "VERSION",       PHP_ION_VERSION);
    PION_CLASS_CONST_LONG(ION, "VERSION_NUMBER",  PHP_ION_VERSION_NUMBER);
    PION_CLASS_CONST_LONG(ION, "ONCE",            EVLOOP_ONCE);
    PION_CLASS_CONST_LONG(ION, "NONBLOCK",        EVLOOP_NONBLOCK);
//    PION_CLASS_CONST_LONG(ION_Deferred, "FAILED",    DEFERRED_FAILED);
//    PION_CLASS_CONST_LONG(ION_Deferred, "FINISHED",  DEFERRED_FINISHED);
//    PION_CLASS_CONST_LONG(ION_Deferred, "INTERNAL",  DEFERRED_INTERNAL);
//    PION_CLASS_CONST_LONG(ION_Deferred, "TIMED_OUT", DEFERRED_TIMED_OUT);
//    PION_CLASS_CONST_LONG(ION_Deferred, "REJECTED",  DEFERRED_REJECTED);
//
//    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_RejectException, Exception, "ION\\Deferred\\RejectException", NULL);
//    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_TimeoutException, ION_Deferred_RejectException, "ION\\Deferred\\TimeoutException", NULL);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION) {
    return SUCCESS;
}