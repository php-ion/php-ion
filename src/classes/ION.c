#include "ion.h"
#include "config.h"

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
        zend_throw_error(NULL, ERR_ION_DISPATCH_IN_LOOP, 1);
        return;
    }

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END();

    GION(flags) |= ION_LOOP_STARTED;
    ret = event_base_loop(GION(base), (int)flags);
    GION(flags) &= ~ION_LOOP_STARTED;

    if(ret == -1) {
        zend_throw_error(NULL, ERR_ION_DISPATCH_FAILED, 1);
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
    }
}

METHOD_ARGS_BEGIN(ION, stop, 0)
    METHOD_ARG_DOUBLE(timeout, 0)
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
        zend_string_release(interval->name);
    }
    if(interval->promisor) {
        ion_object_release(interval->promisor);
        ion_promisor_remove_object(interval->promisor);
    }
    efree(interval);
}

static void _ion_interval_invoke(evutil_socket_t fd, short flags, void * arg) {
    ION_CB_BEGIN();
    ion_interval * interval = (ion_interval *) arg;

    if(interval->name) {
        ion_promisor_done_string(interval->promisor, interval->name, 1);
    } else {
        ion_promisor_done_true(interval->promisor);
    }
    if(interval->repeat) {
        if(event_add(interval->timer, &interval->tv) == FAILURE) {
            ion_promisor_throw(interval->promisor, ion_ce_ION_RuntimeException, ERR_ION_AWAIT_FAILED, 0);
            _ion_interval_free(interval);
        }
    } else {
        _ion_interval_free(interval);
    }
    ION_CB_END();
}

static void _ion_interval_dtor(zval * dest) {
    _ion_interval_free(Z_PTR_P(dest));
}

static zval _ion_interval_cancel(ion_promisor * promise, zval * ex) {
    zval ret;
    ZVAL_UNDEF(&ret);
    _ion_interval_free(Z_PTR_P(&promise->object));
    zend_throw_exception_object(ex);

    return ret;
}

ion_promisor * _ion_timer_ctor(double timeout, zend_bool repeat, zend_string * name) {
    ion_interval * interval;
    ion_promisor * promisor;
    interval = ecalloc(1, sizeof(ion_interval));
    interval->repeat = repeat;
    if(repeat) {
        promisor = ion_promisor_sequence_new(NULL);
    } else {
        promisor = ion_promisor_deferred_new_ex(_ion_interval_cancel);
    }
    interval->promisor   = promisor;
    interval->tv.tv_usec = ((int)(timeout*1000000) % 1000000);
    interval->tv.tv_sec  = (int)timeout;
    if(name) {
        // @todo check name in GION(timers)
        interval->name   = zend_string_copy(name);
    }
    interval->timer      = event_new(GION(base), -1, EV_TIMEOUT, _ion_interval_invoke, interval);

    if(interval->name) {
        zend_hash_add_ptr(GION(timers), interval->name, interval);
    }

    if(event_add(interval->timer, &interval->tv) == FAILURE) {
        _ion_interval_free(interval);
        zend_throw_exception(ion_ce_ION_RuntimeException, ERR_ION_AWAIT_FAILED, 0);
        return NULL;
    } else {
        ion_object_addref(interval->promisor);
        ion_promisor_set_object_ptr(interval->promisor, interval, NULL);
        return promisor;
    }
}

/** public function ION::await(double $time) : ION\Deferred */
CLASS_METHOD(ION, await) {
    ion_promisor * deferred;
    double timeout = 0.0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(timeout)
    ZEND_PARSE_PARAMETERS_END();
    if(timeout < 0) {
        zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_AWAIT_INVALID_TIME, 0);
        return;
    }
    deferred = _ion_timer_ctor(timeout, false, NULL);
    if(deferred) {
        RETURN_OBJ(ION_OBJECT_ZOBJ(deferred));
    }
}

METHOD_ARGS_BEGIN(ION, await, 1)
    METHOD_ARG_FLOAT(time, 0)
METHOD_ARGS_END()

/** public function ION::interval(double $time, string $name = NULL) : ION\Sequence */
CLASS_METHOD(ION, interval) {
    double           timeout = 0.0;
    zend_string    * name = NULL;
    ion_promisor   * sequence;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_DOUBLE(timeout)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);
    if(timeout < 0) {
        zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_AWAIT_INVALID_TIME, 0);
        return;
    }
    sequence = _ion_timer_ctor(timeout, true, ZSTR_LEN(name) ? name : NULL);
    if(sequence) {
        RETURN_OBJ(ION_OBJECT_ZOBJ(sequence));
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
    zval         * resolver = NULL;
    ion_promisor * promise = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(resolver)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if (Z_TYPE_P(resolver) == IS_OBJECT) {
        if (Z_ISPROMISE_P(resolver)) {
            RETURN_ZVAL(resolver, 1, 0);
        }
        if (instanceof_function(Z_OBJCE_P(resolver), zend_ce_generator)) {
            // todo
        }
    }
    if (zend_is_callable(resolver, IS_CALLABLE_CHECK_SILENT, NULL)) {
        promise = ion_promisor_promise_new(resolver, NULL);
        ion_promisor_done_null(promise);
    } else {
        promise = ion_promisor_promise_new(NULL, NULL);
        ion_promisor_done(promise, resolver);
    }
    RETURN_OBJ(ION_OBJECT_ZOBJ(promise));
}

METHOD_ARGS_BEGIN(ION, promise, 1)
    METHOD_ARG(resolver, 0)
METHOD_ARGS_END()

/** public function ION::getStats(bool $reset = true) : array */
CLASS_METHOD(ION, getStats) {
    zend_bool reset = 1;
    double reset_ts;

    ZEND_PARSE_PARAMETERS_START(0, 1)
            Z_PARAM_OPTIONAL
            Z_PARAM_BOOL(reset)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    array_init(return_value);
    add_assoc_double(return_value, "php_time", GION(php_time));
    add_assoc_long(return_value,   "calls_count", GION(calls_count));
    reset_ts = GION(reset_ts).tv_sec;
    reset_ts += GION(reset_ts).tv_usec / 1e6;
    add_assoc_double(return_value, "reset_ts", reset_ts);

    if(reset) {
        GION(php_time) = 0.0;
        GION(calls_count) = 0;
        gettimeofday(&GION(reset_ts), NULL);
    }
}

METHOD_ARGS_BEGIN(ION, getStats, 0)
    ARGUMENT(reset, _IS_BOOL)
METHOD_ARGS_END()


CLASS_METHODS_START(ION)
    METHOD(ION, reinit,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, dispatch,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, stop,           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, await,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, interval,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, cancelInterval, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, promise,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION, getStats,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

ZEND_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("ion.metrics", "On", PHP_INI_SYSTEM, OnUpdateBool, define_metrics, zend_ion_globals, ion_globals)
    STD_PHP_INI_BOOLEAN("ion.stats",   "On", PHP_INI_USER, OnUpdateBool, stats_enabled, zend_ion_globals, ion_globals)
ZEND_INI_END()

PHP_MINIT_FUNCTION(ION) {
    REGISTER_INI_ENTRIES();
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

    PION_CLASS_CONST_LONG(ION, "PRIORITY_LOW",       5);
    PION_CLASS_CONST_LONG(ION, "PRIORITY_DEFAULT",   3);
    PION_CLASS_CONST_LONG(ION, "PRIORITY_HIGH",      1);
    PION_CLASS_CONST_LONG(ION, "PRIORITY_URGENT",    0);

    PION_CLASS_CONST_LONG(ION, "INTERVAL_PROTECTED",  1);
    PION_CLASS_CONST_LONG(ION, "INTERVAL_PERSISTENT", 2);
#ifdef ION_DEBUG
    PION_CLASS_CONST_LONG(ION, "DEBUG",              1);
#else
    PION_CLASS_CONST_LONG(ION, "DEBUG",              0);
#endif
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION) {
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION) {
    ALLOC_HASHTABLE(GION(timers));
    zend_hash_init(GION(timers), 128, NULL, NULL, 0);
    GION(queue) = ion_deferred_queue_init();
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION) {
    GION(timers)->pDestructor = _ion_interval_dtor;
    zend_hash_clean(GION(timers));
    zend_hash_destroy(GION(timers));
    FREE_HASHTABLE(GION(timers));
    ion_deferred_queue_free(GION(queue));
    return SUCCESS;
}