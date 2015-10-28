#include <php.h>
#include <event.h>

#include "ION.h"

zend_class_entry * ion_ce_ION;
zend_object_handlers ion_oh_ION;
ion_base *ionBase;
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
    zend_long flags = 0;
    ZEND_PARSE_PARAMETERS_START(0,1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END();

    ion_reinit(flags);
}
METHOD_ARGS_BEGIN(ION, reinit, 1)
    METHOD_ARG(flags, 0)
METHOD_ARGS_END()

/** public function ION::dispatch(int $flags = 0) : bool */
CLASS_METHOD(ION, dispatch) {
    zend_long flags = 0;
    int ret;

    if(ION(flags) & ION_IN_LOOP) {
        zend_throw_error(NULL, "Dispatching in progress", 1);
        return;
    }

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END();

    ION(flags) |= ION_IN_LOOP;
    ret = event_base_loop(ION(base), (int)flags);
    ION(flags) &= ~ION_IN_LOOP;

    if(ret == -1) {
        zend_throw_error(NULL, "Dispatching runtime error", 1);
        return;
    }

    if(ret) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

METHOD_ARGS_BEGIN(ION, dispatch, 1)
    METHOD_ARG_LONG(flags, 0)
METHOD_ARGS_END()

/** public function ION::stop(double $timeout = -1) : self */
CLASS_METHOD(ION, stop) {
    double timeout = -1.0;
    struct timeval time;

    ZEND_PARSE_PARAMETERS_START(0,1)
        Z_PARAM_OPTIONAL
        Z_PARAM_DOUBLE(timeout)
    ZEND_PARSE_PARAMETERS_END();

    if(timeout > 0) {
        time.tv_usec = ((int)(timeout*1000000) % 1000000);
        time.tv_sec = (int)timeout;
        event_base_loopexit(ION(base), &time);
    } else {
        event_base_loopbreak(ION(base));
    }
}

METHOD_ARGS_BEGIN(ION, stop, 0)
    METHOD_ARG_DOUBLE(timeout, 0)
METHOD_ARGS_END()

static void _timer_done(evutil_socket_t fd, short flags, void * arg) {
    ion_promisor * deferred = get_object_instance(arg, ion_promisor);
//    zval * zdeferred = (zval * )arg;
    ion_promisor_done_true(&deferred->std);

    ION_CHECK_LOOP();
//    zval_ptr_dtor(&zdeferred);
}

static void _timer_dtor(zend_object * object) {
    ion_promisor * deferred = get_object_instance(object, ion_promisor);
    event * timer = (event *) deferred->object;
    event_del(timer);
    event_free(timer);
//    zval_ptr_dtor(&zdeferred);
}

/** public function ION::await(double $time) : ION\Deferred */
CLASS_METHOD(ION, await) {
    zend_object * deferred;
    double timeout = 0.0;
    struct timeval tv = { 0, 0 };
    PARSE_ARGS("d", &timeout);
    if(timeout < 0) {
        zend_throw_error(ion_class_entry(InvalidArgumentException), "Timeout sould be unsigned");
        return;
    }
    tv.tv_usec = ((int)(timeout*1000000) % 1000000);
    tv.tv_sec = (int)timeout;
    deferred = ion_promisor_deferred_new_ex(NULL);
//    zDeferred = ion_deferred_new_ex(NULL);
    event * timer = event_new(ION(base), -1, EV_TIMEOUT, _timer_done, deferred);
    if(event_add(timer, &tv) == FAILURE) {
        event_del(timer);
        event_free(timer);
        obj_ptr_dtor(deferred);
        zend_throw_error(ion_class_entry(ION_RuntimeException), "Event failed");
        return;
//        ion_deferred_free(zDeferred);
    } else {
        ion_promisor_store(deferred, timer);
        ion_promisor_dtor(deferred, _timer_dtor);
//        ion_deferred_store(zDeferred, timer, _timer_dtor);
        RETURN_OBJ_ADDREF(deferred);
    }

}

METHOD_ARGS_BEGIN(ION, await, 1)
                METHOD_ARG(time, 0)
METHOD_ARGS_END()

/** public function ION::startInterval(double $time, string $name = NULL) : ION\Deferred */
CLASS_METHOD(ION, startInterval) {

}

METHOD_ARGS_BEGIN(ION, startInterval, 1)
    METHOD_ARG_DOUBLE(time, 0)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END()

/** public function ION::stopInterval(string $name) : bool */
CLASS_METHOD(ION, stopInterval) {

}

METHOD_ARGS_BEGIN_RETURN_BOOL(ION, stopInterval, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END()

CLASS_METHODS_START(ION)
    METHOD(ION, reinit,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, dispatch,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, stop,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, await,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, startInterval, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, stopInterval,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION) {
    PION_REGISTER_STATIC_CLASS(ION, "ION");
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