#include "promisor.h"

pion_cb * generator_send    = NULL;
pion_cb * generator_throw   = NULL;
pion_cb * generator_current = NULL;
pion_cb * generator_key     = NULL;
pion_cb * generator_valid   = NULL;
pion_cb * generator_return  = NULL;

zend_class_entry * ion_ce_ION_Promise_CancelException;
zend_object_handlers ion_oh_ION_Promise_CancelException;
zend_class_entry * ion_ce_ION_Promise_TimeoutException;
zend_object_handlers ion_oh_ION_Promise_TimeoutException;

PHP_MINIT_FUNCTION(promisor) {
    PION_REGISTER_VOID_EXTENDED_CLASS(ION_Promise_CancelException, zend_exception_get_default(), "ION\\Promise\\CancelException");
    PION_REGISTER_VOID_EXTENDED_CLASS(ION_Promise_TimeoutException, ion_class_entry(ION_Promise_CancelException), "ION\\Promise\\TimeoutException");
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(promisor) {
    return SUCCESS;
}

PHP_RINIT_FUNCTION(promisor) {
    generator_send    = pion_cb_fetch_method("Generator", "send");
    generator_current = pion_cb_fetch_method("Generator", "current");
    generator_key     = pion_cb_fetch_method("Generator", "key");
    generator_throw   = pion_cb_fetch_method("Generator", "throw");
    generator_valid   = pion_cb_fetch_method("Generator", "valid");
    generator_return  = pion_cb_fetch_method("Generator", "getReturn");
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(promisor) {
    pion_cb_free(generator_send);
    pion_cb_free(generator_current);
    pion_cb_free(generator_key);
    pion_cb_free(generator_throw);
    pion_cb_free(generator_valid);
    pion_cb_free(generator_return);
    return SUCCESS;
}


static zend_always_inline int ion_promisor_invoke(ion_promisor * promisor, zval * retval, zval * data, int type) {
    int result_type = ION_PROMISOR_DONE;
    if (type & ION_PROMISOR_DONE) {
        if (promisor->done) {
            if((promisor->flags & ION_PROMISOR_HAS_DONE_WITH_FAIL) && pion_cb_required_num_args(promisor->done) == 2) {
                zval helper;
                ZVAL_NULL(&helper);
                // second argument may has incompatible type hint
                if(pion_verify_arg_type(promisor->done, 1, &helper) != SUCCESS) {
                    zend_error(E_NOTICE, "Second argument has incompatible type hint");
                } else if (pion_verify_arg_type(promisor->done, 0, data) == SUCCESS) {
                    *retval = pion_cb_call_with_2_args(promisor->done, data, &helper);
                }
            } else if (pion_cb_num_args(promisor->done)) { // has arguments
                if (pion_verify_arg_type(promisor->done, 0, data) == SUCCESS) {
                    *retval = pion_cb_call_with_1_arg(promisor->done, data);
                }
            } else {
                *retval = pion_cb_call_without_args(promisor->done);
            }
        }
    } else if(!(promisor->flags & ION_PROMISOR_TYPE_DEFERRED) // deferred has no fail callback but has cancel callback
              || (type & ION_PROMISOR_CANCELED && promisor->flags & ION_PROMISOR_TYPE_DEFERRED)) {

        if (promisor->fail) {
            if (pion_verify_arg_type(promisor->fail, 0, data) == SUCCESS) {
                *retval = pion_cb_call_with_1_arg(promisor->fail, data);
            }
        } else if (promisor->flags & ION_PROMISOR_HAS_DONE_WITH_FAIL) {
            if (pion_verify_arg_type(promisor->done, 1, data) == SUCCESS) {
                zval helper;
                ZVAL_NULL(&helper);
                // first argument may has incompatible type hint
                if(pion_verify_arg_type(promisor->done, 0, &helper) != SUCCESS) {
                    zend_error(E_NOTICE, "First argument has incompatible type hint");
                } else {
                    *retval = pion_cb_call_with_2_args(promisor->done, &helper, data);
                }
            }
        } else {
            result_type = ION_PROMISOR_FAILED;
        }
    }

    return result_type;
}

static zend_always_inline void ion_promisor_release(zend_object * promisor_obj) {
    ion_promisor * promisor = get_object_instance(promisor_obj, ion_promisor);
    if(promisor->dtor) {
        promisor->dtor(promisor_obj);
        promisor->dtor = NULL;
    }
    if(promisor->done) {
        pion_cb_free(promisor->done);
        promisor->done = NULL;
    }
    if(promisor->fail) {
        pion_cb_free(promisor->fail);
        promisor->fail = NULL;
    }
    if(promisor->progress) {
        pion_cb_free(promisor->progress);
        promisor->progress = NULL;
    }
}


void ion_promisor_resolve(zend_object * promise_obj, zval * data, int type) {
    ion_promisor * promise = get_object_instance(promise_obj, ion_promisor);
    zval           zpromise;
    zval           retval;
    zval           result;
    int            result_type = 0;
    zend_bool      resolved;
    zend_class_entry * object_ce = NULL;

    ZVAL_OBJ(&zpromise, promise_obj);
    Z_ADDREF(zpromise);
    ZVAL_UNDEF(&retval);
    if(promise->await) {
        result_type = type & ION_PROMISOR_FINISHED;
        ZVAL_COPY(&result, data);
        obj_ptr_dtor(promise->await);
        promise->await = NULL;
        goto watch_result;
    } else {
        result_type = ion_promisor_invoke(promise, &retval, data, type);
    }

    if(Z_ISUNDEF(retval)) {
        if(EXPECTED(EG(exception))) {
            ZVAL_OBJ(&result, EG(exception));
            EG(exception) = NULL;
            result_type = ION_PROMISOR_FAILED;
        } else {
            ZVAL_COPY(&result, data);
            result_type = type & ION_PROMISOR_FINISHED;
        }
        resolved = 1;
    } else {
        result = retval;
        watch_result:
        resolved = 1;

        if(Z_TYPE(result) == IS_OBJECT) {
            object_ce = Z_OBJCE(result);
            if(object_ce == ion_class_entry(Generator)) {
                if(promise->generator) { // push the generator to stack
                    PION_ARRAY_PUSH(promise->generators_stack, promise->generators_count, promise->generator);
                }
                promise->generator = Z_OBJ(result);
                result = pion_cb_obj_call_without_args(generator_current, promise->generator);
                goto watch_result;
            } else if(object_ce == ion_class_entry(ION_Promise)
                      || object_ce == ion_class_entry(ION_Deferred)
                      || instanceof_function(object_ce, ion_class_entry(ION_Promise))) {
                ion_promisor * await = get_instance(&result, ion_promisor);
                if(await->flags & ION_PROMISOR_FINISHED) {
                    ZVAL_COPY(&result, &await->result);
                    result_type = await->flags & ION_PROMISOR_FINISHED;
//                    obj_ptr_dtor(await);
                    goto watch_result;
                } else {
                    promise->await = Z_OBJ(result);
                    PION_ARRAY_PUSH(await->handlers, await->handler_count, promise_obj);
                    zval_add_ref(&zpromise);
                    resolved = 0;
                }
            }
        } else {
            resolved = 1;
        }
        if(promise->generator && resolved) {
            resume_generator: {
                zval             next;
                zval             is_valid;
                zend_bool        is_valid_generator = 0;
                zend_generator * generator = get_object_instance(promise->generator, zend_generator);

                if(result_type == ION_PROMISOR_DONE) {
                    next = pion_cb_obj_call_with_1_arg(generator_send, promise->generator, &result);
                } else {
                    next = pion_cb_obj_call_with_1_arg(generator_throw, promise->generator, &result);
                }
                zval_ptr_dtor(&result);
                if(EG(exception)) {
                    ZVAL_OBJ(&result, EG(exception));
                    EG(exception) = NULL;
                    result_type = ION_PROMISOR_FAILED;
                    is_valid_generator = 0;
                } else {

                    result = next;
                    is_valid = pion_cb_obj_call_without_args(generator_valid, promise->generator);
                    if(Z_ISFALSE(is_valid)) {
                        is_valid_generator = 0;
                        zval_ptr_dtor(&result);
                        result_type = ION_PROMISOR_DONE;

                        if(Z_ISUNDEF(generator->retval)) {
                            ZVAL_NULL(&result);
                        } else {
                            ZVAL_COPY(&result, &generator->retval);
                        }
                    } else {
                        is_valid_generator = 1;
                    }
                }

                if(!is_valid_generator) {
                    obj_ptr_dtor(promise->generator);
                    PION_ARRAY_POP(promise->generators_stack, promise->generators_count, promise->generator);
                    if(promise->generator) {
                        goto resume_generator;
                    }
                }
                goto watch_result;
            }
        }
    }
    if(resolved) {
        promise->result = result;
        promise->flags |= result_type;
        ion_promisor_release(promise_obj);
        if(promise->handler_count) {
            for(ushort i = 0; i < promise->handler_count; i++) {
                ion_promisor * handler = get_object_instance(promise->handlers[i], ion_promisor);
                ion_promisor_resolve(promise->handlers[i], &result, result_type);
                obj_ptr_dtor(promise->handlers[i]);
                promise->handlers[i] = NULL;
            }

            efree(promise->handlers);
            promise->handlers = NULL;
            promise->handler_count = 0;
        }
    }
    zval_ptr_dtor(&zpromise);
}

void ion_promisor_sequence_invoke(zend_object * promise, zval * args) {
    zend_object * clone = ion_promisor_clone(promise);
    ion_promisor_done(clone, args);
    obj_ptr_dtor(clone);
}

zend_object * ion_promisor_promise_new(zval * done, zval * fail, zval * progress) {
    return pion_new_object_arg_3(ion_class_entry(ION_Promise), done, fail, progress);
}


zend_object * ion_promisor_sequence_new(zval * init) {
    if(init) {
        return pion_new_object_arg_1(ion_class_entry(ION_Sequence), init);
    } else {
        return pion_new_object_arg_0(ion_class_entry(ION_Sequence));
    }
}

zend_object * ion_promisor_deferred_new(zval * canceler) {
    return pion_new_object_arg_1(ion_class_entry(ION_Deferred), canceler);
}

zend_object * ion_promisor_deferred_new_ex(promisor_canceler_t canceler) {
    zval object;
    ion_promisor * deferred;

    object_init_ex(&object, ion_class_entry(ION_Deferred));
    deferred = get_instance(&object, ion_promisor);
    deferred->flags |= ION_PROMISOR_INTERNAL;
    if(canceler) {
        deferred->canceler = canceler;
    }

    return &deferred->std;
}



int ion_promisor_set_callbacks(zend_object * promise_obj, zval * done, zval * fail, zval * progress) {
    ion_promisor * promise = get_object_instance(promise_obj, ion_promisor);
    if(done) {
        promise->done = pion_cb_create_from_zval(done);
        promise->flags |= ION_PROMISOR_HAS_DONE;

        if(pion_cb_num_args(promise->done) > 1) {
            promise->flags |= ION_PROMISOR_HAS_DONE_WITH_FAIL;
        }
    }
    if(fail) {
        promise->fail = pion_cb_create_from_zval(fail);
        promise->flags |= ION_PROMISOR_HAS_FAIL;
    }
    if(progress) {
        promise->progress = pion_cb_create_from_zval(progress);
        promise->flags |= ION_PROMISOR_HAS_PROGRESS;
    }
    return SUCCESS;
}

zend_object * ion_promisor_push_callbacks(zend_object * promise_obj, zval * on_done, zval * on_fail, zval * on_progress) {
    ion_promisor * promisor = get_object_instance(promise_obj, ion_promisor);
    zend_object * handler;
    zval zpromisor;

    if(promisor->flags & ION_PROMISOR_FINISHED) {
        handler = ion_promisor_promise_new(on_done, on_fail, on_progress);
        ion_promisor_resolve(handler, &promisor->result, promisor->flags & ION_PROMISOR_FINISHED);
    } else {
        object_init_ex(&zpromisor, ion_class_entry(ION_Promise));
        handler = Z_OBJ(zpromisor);
        ion_promisor_set_callbacks(handler, on_done, on_fail, on_progress);
        PION_ARRAY_PUSH(promisor->handlers, promisor->handler_count, handler);
    }
    return handler;
}

void ion_promisor_cancel(zend_object * promisor_obj, const char *message) {
    zval           value;
    ion_promisor * promisor = get_object_instance(promisor_obj, ion_promisor);
    zend_object  * error = pion_exception_new(ion_class_entry(ION_Promise_CancelException), message, 0);
    OBJ_ADDREF(promisor_obj);
    promisor->flags |= ION_PROMISOR_CANCELED;
    if(promisor->canceler) {
        promisor->canceler(promisor_obj);
        if(EG(exception)) {
            zend_exception_set_previous(EG(exception), error);
            error = EG(exception);
            EG(exception) = NULL;
        }
    }
    ZVAL_OBJ(&value, error);
    ion_promisor_resolve(promisor_obj, &value, ION_PROMISOR_FAILED | ION_PROMISOR_CANCELED);
    zval_ptr_dtor(&value);
    OBJ_DELREF(promisor_obj);
}


zend_object * ion_promisor_clone(zend_object * proto_obj) {
    ion_promisor * proto = get_object_instance(proto_obj, ion_promisor);
    ion_promisor * clone = emalloc(sizeof(ion_promisor));
    memset(clone, 0, sizeof(ion_promisor));
    clone->flags = proto->flags & ~ION_PROMISOR_PROTOTYPE;
    ZVAL_UNDEF(&clone->result);
    zend_object_std_init(&clone->std, proto->std.ce);
    object_properties_init(&clone->std, proto->std.ce);
    clone->std.handlers = proto->std.handlers;
    zend_object * clone_obj = &clone->std;
    zend_objects_clone_members(clone_obj, proto_obj);

    if(proto->flags & ION_PROMISOR_INTERNAL) {
        zend_throw_error(NULL, "Trying to clone an internal promisor", 0);
        return clone_obj;
    }
    if(proto->await || proto->generator || proto->generators_count) {
        zend_throw_error(NULL, "Promisor in progress", 0);
        return clone_obj;
    }
    if(proto->dtor) {
        clone->dtor = proto->dtor;
    }
    if(proto->done) {
        clone->done = pion_cb_dup(proto->done);
    }
    if(proto->fail) {
        clone->fail = pion_cb_dup(proto->fail);
    }
    if(proto->progress) {
        clone->progress = pion_cb_dup(proto->progress);
    }

    if(proto->handler_count) {
        clone->handlers = emalloc(sizeof(zend_object) * proto->handler_count);
        for(uint i=0; i<proto->handler_count; i++) {
            clone->handlers[i] = ion_promisor_clone(proto->handlers[i]);
        }
        clone->handler_count = proto->handler_count;
    }
    if(!Z_ISUNDEF(proto->result)) {
        ZVAL_COPY(&clone->result, &proto->result);
    }
    return clone_obj;
}

zend_object * ion_promisor_clone_obj(zval * zobject) {
    return ion_promisor_clone(Z_OBJ_P(zobject));
}

void ion_promisor_free(zend_object * promisor_obj) {
    ion_promisor * promisor = get_object_instance(promisor_obj, ion_promisor);

    zend_object_std_dtor(promisor_obj);
    ion_promisor_release(promisor_obj);
    if(promisor->await) {
        obj_ptr_dtor(promisor->await);
        promisor->await = NULL;
    }
    if(promisor->generator) {
        obj_ptr_dtor(promisor->generator);
        promisor->generator = NULL;
    }
    if(promisor->generators_count) {
        for(uint i=0; i<promisor->generators_count; i++) {
            obj_ptr_dtor(promisor->generators_stack[i]);
        }
        efree(promisor->generators_stack);
        promisor->generators_count = 0;
    }
    if(promisor->handler_count) {

        for(uint i=0; i<promisor->handler_count; i++) {
            obj_ptr_dtor(promisor->handlers[i]);
        }
        efree(promisor->handlers);
        promisor->handler_count = 0;
    }
    zval_ptr_dtor(&promisor->result);
}