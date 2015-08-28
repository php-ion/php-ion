#include <php.h>
#include <event.h>

#include "ION.h"

//#define event struct event

typedef struct event ev;

DEFINE_CLASS(ION);
IONBase *ionBase;
void ion_reinit(long flags) {
    IONF("Reinit event loop: %d. Cleanup exec events...", (int)flags);
    //zend_hash_clean(ION(execs));



    if(!(flags & PRESERVE_TIMERS)) {
        IONF("Cleanup timer events...");
//        zend_hash_clean(ION(timers));
    }

    if(!(flags & PRESERVE_SIGNALS)) {
        IONF("Cleanup signal events...");
//        zend_hash_clean(ION(signals));
    }

    if(flags & RECREATE_BASE) {
        event_base_free(ION(base));
        ION(base) = event_base_new();
    } else if(event_reinit(ION(base)) == FAILURE) {
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
    METHOD_ARG(flags, 0)
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

METHOD_ARGS_BEGIN(ION, stop, 0)
    METHOD_ARG(timeout, 0)
METHOD_ARGS_END()

static void _timer_done(evutil_socket_t fd, short flags, void * arg) {
    zval * zdeferred = (zval * )arg;
    zval * zresult = NULL;
    TSRMLS_FETCH();
    MAKE_STD_ZVAL(zresult);
    ion_deferred_done(zdeferred, zresult);
    zval_ptr_dtor(&zresult);

    ION_CHECK_LOOP();
//    zval_ptr_dtor(&zdeferred);
}

static void _timer_dtor(void * object, zval * zdeferred TSRMLS_DC) {
    ev * timer = (ev *) object;
    event_del(timer);
    event_free(timer);
    zval_ptr_dtor(&zdeferred);
}

/** public function ION::await(double $time) : ION\Deferred */
CLASS_METHOD(ION, await) {
    zval *zDeferred = NULL;
    double timeout = 0.0;
    struct timeval tv = { 0, 0 };
    PARSE_ARGS("d", &timeout);
    if(timeout < 0) {
        ThrowRuntime("timeout sould be unsigned", 1);
        return;
    }
    tv.tv_usec = (int)((int)(timeout*1000000) % 1000000);
    tv.tv_sec = (int)timeout;
    zDeferred = ion_deferred_new_ex(NULL);
    ev * timer = event_new(ION(base), -1, EV_TIMEOUT, _timer_done, zDeferred);
    if(event_add(timer, &tv) == FAILURE) {
        event_del(timer);
        event_free(timer);
        ion_deferred_free(zDeferred);
    } else {
        ion_deferred_store(zDeferred, timer, _timer_dtor);
        RETURN_ZVAL(zDeferred, 1, 0);
    }

}

METHOD_ARGS_BEGIN(ION, await, 1)
    METHOD_ARG(time, 0)
METHOD_ARGS_END()

CLASS_METHODS_START(ION)
    METHOD(ION, reinit,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, dispatch, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, stop,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, await,    ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION) {
    PION_REGISTER_PLAIN_CLASS(ION, "ION");
    PION_CLASS_CONST_STRING(ION, "VERSION",        ION_VERSION);
    PION_CLASS_CONST_STRING(ION, "ENGINE",         ION_EVENT_ENGINE);
    PION_CLASS_CONST_STRING(ION, "ENGINE_VERSION", event_get_version());
    PION_CLASS_CONST_LONG(ION, "LOOP_ONCE",        EVLOOP_ONCE);
    PION_CLASS_CONST_LONG(ION, "LOOP_NONBLOCK",    EVLOOP_NONBLOCK);
//    PION_CLASS_CONST_LONG(ION, "RECREATE_BASE",    RECREATE_BASE);

    PION_CLASS_CONST_LONG(ION, "EV_READ",            EV_READ);
    PION_CLASS_CONST_LONG(ION, "EV_WRITE",           EV_WRITE);
    PION_CLASS_CONST_LONG(ION, "EV_TIMEOUT",         EV_TIMEOUT);
    PION_CLASS_CONST_LONG(ION, "EV_SIGNAL",          EV_SIGNAL);
    PION_CLASS_CONST_LONG(ION, "EV_PERSIST",         EV_PERSIST);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION) {
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION) {

    return SUCCESS;
}