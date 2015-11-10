#include "promisor.h"

pion_cb * generator_send    = NULL;
pion_cb * generator_throw   = NULL;
pion_cb * generator_current = NULL;
pion_cb * generator_key     = NULL;
pion_cb * generator_valid   = NULL;

zend_class_entry * ion_ce_ION_Promise_CancelException;
zend_object_handlers ion_oh_ION_Promise_CancelException;
zend_class_entry * ion_ce_ION_Promise_TimeoutException;
zend_object_handlers ion_oh_ION_Promise_TimeoutException;

#define Z_ISGENERATOR(zv) (Z_TYPE(zv) == IS_OBJECT && Z_OBJCE(result) == ion_class_entry(Generator))

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
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(promisor) {
    pion_cb_free(generator_send);
    pion_cb_free(generator_current);
    pion_cb_free(generator_key);
    pion_cb_free(generator_throw);
    pion_cb_free(generator_valid);
    return SUCCESS;
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
    zend_bool      skip_generator = 0;
    zend_class_entry * object_ce = NULL;
    pion_cb      * callback = NULL;

    ZVAL_OBJ(&zpromise, promise_obj);
    Z_ADDREF(zpromise);
    ZVAL_UNDEF(&retval);
    if(promise->await) {
        result_type = type & ION_PROMISOR_FINISHED;
        ZVAL_COPY(&result, data);
        zend_object_release(promise->await);
        promise->await = NULL;
        if(promise->generator) {
            resolved = 0;
        } else {
            resolved = 1;
        }
    } else {
        if(type & ION_PROMISOR_DONE) {
            callback = promise->done;
        } else if(type & ION_PROMISOR_CANCELED) {
            if(promise->flags & ION_PROMISOR_TYPE_DEFERRED) {
                callback = promise->fail;
            }
        } else if(type & ION_PROMISOR_FAILED) {
            if(!(promise->flags & ION_PROMISOR_TYPE_DEFERRED)) {
                callback = promise->fail;
            }
        } else if(type & ION_PROMISOR_PROGRESS) {
            callback = promise->progress;
        }
        if(callback) {
            if(pion_cb_required_num_args(callback) > 1) {
                callback = NULL;
            }
            else if(pion_cb_num_args(callback) && pion_verify_arg_type(callback, 0, data) == false) {
                callback = NULL;
            }
        }
        if(callback) {
            zval_add_ref(data);
            retval = pion_cb_call_with_1_arg(callback, data);
            zval_ptr_dtor(data);
            if(EG(exception)) {
                ZEND_ASSERT(Z_ISUNDEF(retval));
                ZVAL_OBJ(&result, EG(exception));
                zval_add_ref(&result);
                zend_clear_exception();
                result_type = ION_PROMISOR_FAILED;
                resolved = 1;
            } else {
                result = retval;
                if(Z_ISGENERATOR(result) && !pion_cb_is_generator(callback)) { // generator returned
                    skip_generator = 1;
                }
                result_type = ION_PROMISOR_DONE;
                resolved = 0;
            }
        } else {
            ZVAL_COPY(&result, data);
            result_type = type & ION_PROMISOR_FINISHED;
            resolved = 1;
        }
    }


    if(!resolved) {
        watch_result: {
            resolved = 1;

            if (Z_TYPE(result) == IS_OBJECT) {
                object_ce = Z_OBJCE(result);
                if (object_ce == ion_class_entry(Generator)) {
                    if(skip_generator || promise->generator) {
                        resolved = 1;
                    } else {
                        promise->generator = Z_OBJ(result);
                        result = pion_cb_obj_call_without_args(generator_current, promise->generator);
                        goto watch_result;
                    }
                } else if (object_ce == ion_class_entry(ION_Promise)
                           || object_ce == ion_class_entry(ION_Deferred)
                           || instanceof_function(object_ce, ion_class_entry(ION_Promise))) {
                    ion_promisor * await = get_instance(&result, ion_promisor);
                    if (await->flags & ION_PROMISOR_FINISHED) {
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
        }
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
                ZEND_ASSERT(Z_ISUNDEF(next));
                ZVAL_OBJ(&result, EG(exception));
                zval_add_ref(&result);
                zend_clear_exception();
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
                    skip_generator = 1;
                } else {
                    is_valid_generator = 1;
                    if(Z_ISGENERATOR(result)) {
                        goto resume_generator;
                    }
                }
            }

            if(!is_valid_generator) {
                zend_object_release(promise->generator);
                promise->generator = NULL;
                promise->flags |= ION_PROMISOR_RESOLVED;
            }
            goto watch_result;
        }
    }

    if(resolved) {
        promise->result = result;
        promise->flags |= result_type;
        ion_promisor_release(promise_obj);
        if(promise->handler_count) {
            for(ushort i = 0; i < promise->handler_count; i++) {
                ion_promisor_resolve(promise->handlers[i], &result, result_type);
                zend_object_release(promise->handlers[i]);
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
    zend_object_release(clone);
}

zend_object * ion_promisor_promise_new(zval * done, zval * fail, zval * progress) {
    zval object;
    ion_promisor * promise;

    object_init_ex(&object, ion_class_entry(ION_Promise));
    promise = get_instance(&object, ion_promisor);
    promise->flags |= ION_PROMISOR_INTERNAL;
    ion_promisor_set_callbacks(&promise->std, done, fail, progress);
    return &promise->std;
//    return pion_new_object_arg_3(ion_class_entry(ION_Promise), done, fail, progress);
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
        if(!promise->done) {
            return FAILURE;
        }
        promise->flags |= ION_PROMISOR_HAS_DONE;

        if(pion_cb_required_num_args(promise->done) > 1) {
            return FAILURE;
        }
    }
    if(fail) {
        promise->fail = pion_cb_create_from_zval(fail);
        if(!promise->fail) {
            return FAILURE;
        }
        promise->flags |= ION_PROMISOR_HAS_FAIL;
    }
    if(progress) {
        promise->progress = pion_cb_create_from_zval(progress);
        if(!promise->progress) {
            return FAILURE;
        }
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
        if(ion_promisor_set_callbacks(handler, on_done, on_fail, on_progress) == FAILURE) {
            return NULL;
        }
        PION_ARRAY_PUSH(promisor->handlers, promisor->handler_count, handler);
        obj_add_ref(handler);
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
    zend_object_release(promisor_obj);
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
        zend_object_release(promisor->await);
        promisor->await = NULL;
    }
    if(promisor->generator) {
        zend_object_release(promisor->generator);
        promisor->generator = NULL;
    }
    if(promisor->generators_count) {
        for(uint i=0; i<promisor->generators_count; i++) {
            zend_object_release(promisor->generators_stack[i]);
        }
        efree(promisor->generators_stack);
        promisor->generators_count = 0;
    }
    if(promisor->handler_count) {

        for(uint i=0; i<promisor->handler_count; i++) {
            zend_object_release(promisor->handlers[i]);
        }
        efree(promisor->handlers);
        promisor->handler_count = 0;
    }
    zval_ptr_dtor(&promisor->result);
}