#include <php.h>
#include <event.h>
#include <signal.h>

#include "ION.h"

zend_class_entry * ion_ce_ION;
zend_object_handlers ion_oh_ION;

/** public function ION::reinit() : bool */
CLASS_METHOD(ION, reinit) {
    if(event_reinit(GION(base)) == SUCCESS) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION, reinit);

/** public function ION::dispatch(int $flags = 0) : bool */
CLASS_METHOD(ION, dispatch) {
    zend_long flags = 0;
    int ret;

    if(GION(flags) & ION_LOOP_STARTED) {
        zend_throw_error(NULL, "Dispatching in progress", 1);
        return;
    }

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END();

    GION(flags) |= ION_LOOP_STARTED;
    ret = event_base_loop(GION(base), (int)flags);
//    ret = event_base_loop(GION(base), EVLOOP_NONBLOCK | EVLOOP_ONCE);
    GION(flags) &= ~ION_LOOP_STARTED;

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
        event_base_loopexit(GION(base), &time);
    } else {
        event_base_loopbreak(GION(base));
//        event_base_loopexit(ION(base), NULL);
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
    ion_event * timer = (ion_event *) deferred->object;
    event_del(timer);
    event_free(timer);
    obj_ptr_dtor(object);
//    zval_ptr_dtor(&zdeferred);
}

/** public function ION::await(double $time) : ION\Deferred */
CLASS_METHOD(ION, await) {
    zend_object * deferred;
    double timeout = 0.0;
    struct timeval tv = { 0, 0 };
    ion_event * timer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(timeout)
    ZEND_PARSE_PARAMETERS_END();
    if(timeout < 0) {
        zend_throw_exception(ion_class_entry(InvalidArgumentException), "Timeout sould be unsigned", 0);
        return;
    }
    tv.tv_usec = ((int)(timeout*1000000) % 1000000);
    tv.tv_sec = (int)timeout;
    deferred = ion_promisor_deferred_new_ex(NULL);
    timer = event_new(GION(base), -1, EV_TIMEOUT, _timer_done, deferred);
    if(event_add(timer, &tv) == FAILURE) {
        event_del(timer);
        event_free(timer);
        obj_ptr_dtor(deferred);
        zend_throw_exception(ion_class_entry(ION_RuntimeException), "Unable to add event to queue", 0);
        return;
    } else {
        ion_promisor_store(deferred, timer);
        ion_promisor_dtor(deferred, _timer_dtor);
        RETURN_OBJ_ADDREF(deferred);
    }

}

METHOD_ARGS_BEGIN(ION, await, 1)
    METHOD_ARG_FLOAT(time, 0)
METHOD_ARGS_END()

static void _ion_interval_free(ion_interval * interval) {
    if(interval->timer) {
        event_del(interval->timer);
        event_free(interval->timer);
        interval->timer = NULL;
    } else {
        return;
    }
    if(interval->name) {
        int res = zend_symtable_del(GION(timers), interval->name);
        ZEND_ASSERT(res == SUCCESS);
        zend_string_free(interval->name);
    }
    if(interval->promisor) {
        obj_ptr_dtor(interval->promisor);
    }
    efree(interval);
}

static void _ion_interval_invoke(evutil_socket_t fd, short flags, void * arg) {
    ion_interval * interval = (ion_interval *) arg;
    zval data;

    ZVAL_TRUE(&data);
    ion_promisor_sequence_invoke(interval->promisor, &data);
    if(event_add(interval->timer, &interval->tv) == FAILURE) {
        _ion_interval_free(interval);
        zend_throw_exception(ion_class_entry(ION_RuntimeException), "Unable to add event to queue", 0);
    }
    ION_CHECK_LOOP();
}

static void _ion_interval_dtor(zend_object * sequence) {
    ion_promisor * promisor = get_object_instance(sequence, ion_promisor);
    if(promisor->object) {
        _ion_interval_free((ion_interval *) promisor->object);
    }
}

static void _ion_clean_interval(zval * dest) {
    _ion_interval_free(Z_PTR_P(dest));
}


/** public function ION::interval(double $time, string $name = NULL) : ION\Sequence */
CLASS_METHOD(ION, interval) {
    double           timeout = 0.0;
    zend_string    * name = NULL;
    ion_interval   * interval;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_DOUBLE(timeout)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    // todo: check existing interval by name

    interval = ecalloc(1, sizeof(ion_interval));
    interval->promisor   = ion_promisor_sequence_new(NULL);
    interval->tv.tv_usec = ((int)(timeout*1000000) % 1000000);
    interval->tv.tv_sec  = (int)timeout;
    if(name) {
        interval->name   = zend_string_copy(name);
    }
    interval->timer      = event_new(GION(base), -1, EV_TIMEOUT, _ion_interval_invoke, interval);

    if(interval->name) {
        zend_hash_add_ptr(GION(timers), interval->name, interval);
    }

    if(event_add(interval->timer, &interval->tv) == FAILURE) {
        _ion_interval_free(interval);
        zend_throw_exception(ion_class_entry(ION_RuntimeException), "Unable to add event to queue", 0);
        return;
    } else {
        ion_promisor_store(interval->promisor, interval);
        ion_promisor_dtor(interval->promisor, _ion_interval_dtor);
        obj_add_ref(interval->promisor);
        RETURN_OBJ(interval->promisor);
    }
}

METHOD_ARGS_BEGIN(ION, interval, 1)
    METHOD_ARG_DOUBLE(time, 0)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END()

/** public function ION::cancelInterval(string $name) : bool */
CLASS_METHOD(ION, cancelInterval) {
    ion_interval   * interval;
    zend_string    * name = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    interval = zend_hash_find_ptr(GION(timers), name);
    if(interval) {
        _ion_interval_free(interval);
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_ARGS_BEGIN_RETURN_BOOL(ION, cancelInterval, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END()

/** public function ION::promise(mixed $resolver) : ION\Promise */
CLASS_METHOD(ION, promise) {
    zval        * resolver = NULL;
    zend_object * promise = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(resolver)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if (Z_TYPE_P(resolver) == IS_OBJECT) {
        // todo: for sequence
        if(instanceof_function(Z_OBJCE_P(resolver), ion_class_entry(ION_Promise))) {
            RETURN_ZVAL(resolver, 1, 0);
        }
    }
    if (zend_is_callable(resolver, IS_CALLABLE_CHECK_SILENT, NULL)) {
        promise = ion_promisor_promise_new(resolver, NULL, NULL);
        ion_promisor_done_null(promise);
    } else {
        promise = ion_promisor_promise_new(NULL, NULL, NULL);
        ion_promisor_done(promise, resolver);
    }
    RETURN_OBJ(promise);
}

METHOD_ARGS_BEGIN(ION, promise, 1)
    METHOD_ARG(resolver, 0)
METHOD_ARGS_END()

CLASS_METHODS_START(ION)
    METHOD(ION, reinit,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, dispatch,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, stop,           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, await,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, interval,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, cancelInterval, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, promise,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION) {
    PION_REGISTER_STATIC_CLASS(ION, "ION");
    PION_CLASS_CONST_STRING(ION, "VERSION",        ION_VERSION);
    PION_CLASS_CONST_STRING(ION, "ENGINE",         ION_EVENT_ENGINE);
    PION_CLASS_CONST_STRING(ION, "ENGINE_VERSION", event_get_version());
    PION_CLASS_CONST_LONG(ION, "LOOP_ONCE",        EVLOOP_ONCE);
    PION_CLASS_CONST_LONG(ION, "LOOP_NONBLOCK",    EVLOOP_NONBLOCK);

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
    ALLOC_HASHTABLE(GION(timers));
    zend_hash_init(GION(timers), 128, NULL, _ion_clean_interval, 0);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION) {
    zend_hash_clean(GION(timers));
    zend_hash_destroy(GION(timers));
    FREE_HASHTABLE(GION(timers));
    return SUCCESS;
}