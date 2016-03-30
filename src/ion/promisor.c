#include "promisor.h"
#include "debug.h"

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
        promisor->dtor(promisor);
        promisor->dtor = NULL;
    }
    if(promisor->done.type == ION_PROMISOR_CB_PHP) {
        pion_cb_free(promisor->done.cb.php);
        promisor->done.type = ION_PROMISOR_CB_UNSET;
    }
    if(promisor->fail.type == ION_PROMISOR_CB_PHP) {
        pion_cb_free(promisor->fail.cb.php);
        promisor->fail.type = ION_PROMISOR_CB_UNSET;
    }
    if(promisor->canceler.type == ION_PROMISOR_CB_PHP) {
        pion_cb_free(promisor->canceler.cb.php);
        promisor->canceler.type = ION_PROMISOR_CB_UNSET;
    }
}


void ion_promisor_resolve(zend_object * promise_obj, zval * data, uint32_t type) {
    ion_promisor * promise = get_object_instance(promise_obj, ion_promisor);
    zval           zpromise;
    zval           retval;
    zval           result;
    uint32_t       result_type = 0;
    zend_bool      resolved;
    zend_bool      skip_generator = 0;
    ion_promisor_action_cb * callback = NULL;

    if(promise->flags & ION_PROMISOR_FINISHED) {
        return;
    }

    ZVAL_OBJ(&zpromise, promise_obj);
    Z_ADDREF(zpromise);
    ZVAL_UNDEF(&retval);
    ZVAL_UNDEF(&result);
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
        promise->flags |= ION_PROMISOR_PROCESSING;
        if(type & ION_PROMISOR_DONE) {
            if(promise->done.type) {
                callback = &promise->done;
            }
        } else if(type & ION_PROMISOR_CANCELED) {
            if(promise->canceler.type) {
                callback = &promise->canceler;
            }
        } else if(type & ION_PROMISOR_FAILED) {
            if(promise->fail.type) {
                callback = &promise->fail;
            }
        }
        if(callback && callback->type == ION_PROMISOR_CB_PHP) {
//            if(promise->flags & ION_PROMISOR_MULTI_ARGS) {
//                if(type & ION_PROMISOR_MULTI_ARGS) {
//                    uint8_t count = (uint8_t) (type >> ION_PROMISOR_ARGS_NUM_SHIFT);
//                    for(uint8_t i = 0; i < count; i++) {
//                        if(pion_verify_arg_type(callback, i, &data[i]) == false) {
//                            callback = NULL;
//                        }
//                    }
//                }
//            } else {
                if(pion_cb_required_num_args(callback->cb.php) > 1) {
                    callback = NULL;
                } else if(pion_cb_num_args(callback->cb.php) && pion_verify_arg_type(callback->cb.php, 0, data) == false) {
                    callback = NULL;
                }
//            }
        }
        if(callback) {
            zval_add_ref(data);
            if(callback->type == ION_PROMISOR_CB_PHP) {
                retval = pion_cb_call_with_1_arg(callback->cb.php, data);
            } else {
                retval = callback->cb.internal(promise, data);
            }
            zval_ptr_dtor(data);
            if(EG(exception)) {
                ZEND_ASSERT(Z_ISUNDEF(retval));
                ZVAL_OBJ(&result, EG(exception));
                zval_add_ref(&result);
                zend_clear_exception();
                result_type = ION_PROMISOR_FAILED;
                resolved = 1;
            } else if(!Z_ISUNDEF(retval)) {
                result = retval;
                if(Z_ISGENERATOR(result)
                   && callback->type == ION_PROMISOR_CB_PHP
                   && !pion_cb_is_generator(callback->cb.php)) { // generator returned
                    skip_generator = 1;
                }
                result_type = ION_PROMISOR_DONE;
                resolved = 0;
            } else {
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
                if (Z_OBJCE(result) == ion_ce_Generator) {
                    if(skip_generator || promise->generator) {
                        resolved = 1;
                    } else {
                        promise->generator = Z_OBJ(result);
                        result = pion_cb_obj_call_without_args(generator_current, promise->generator);
                        goto watch_result;
                    }
                } else if (Z_ISPROMISE(result)) {
                    ion_promisor * await = get_instance(&result, ion_promisor);
                    if (await->flags & ION_PROMISOR_FINISHED) {
                        ZVAL_COPY(&result, &await->result);
                        result_type = await->flags & ION_PROMISOR_FINISHED;
                        zend_object_release(&await->std);
                        goto watch_result;
                    } else {
                        promise->await = Z_OBJ(result);
                        PION_ARRAY_PUSH(await->handlers, await->handler_count, promise_obj);
                        zval_add_ref(&zpromise);
                        resolved = 0;
                    }
                } else if (Z_OBJCE(result) == ion_ce_ION_Sequence_Quit) {

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
            }
            goto watch_result;
        }
    }

    if(resolved) {
        promise->result = result;
        promise->flags |= result_type;
        promise->flags &= ~ION_PROMISOR_PROCESSING;
        ion_promisor_release(promise_obj);
        if(promise->handler_count) {
            for(ushort i = 0; i < promise->handler_count; i++) {
                zend_object * handler = NULL;
                if(ion_promisor_get_flags(promise->handlers[i]) & ION_PROMISOR_PROTOTYPE) {
                    handler = ion_promisor_clone(promise->handlers[i]);
                    zend_object_release(promise->handlers[i]);
                } else {
                    handler = promise->handlers[i];
                }
                ion_promisor_resolve(handler, &result, result_type);
                zend_object_release(handler);
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

void ion_promisor_sequence_invoke_args(zend_object * promise, zval * args, int count) {
    zend_object * clone = ion_promisor_clone(promise);
    ion_promisor_resolve(clone, args, (uint32_t) (ION_PROMISOR_DONE | ION_PROMISOR_MULTI_ARGS | (count << ION_PROMISOR_ARGS_NUM_SHIFT)));
    zend_object_release(clone);
}

zend_object * ion_promisor_promise_new(zval * done, zval * fail) {
    ion_promisor * promise = ion_promisor_promise_ex(0);

    if(ion_promisor_set_callbacks(ION_OBJ(promise), done, fail) == FAILURE) {
        zend_throw_exception(ion_ce_ION_RuntimeException, "Promise expects a valid callbacks", 0);
        return NULL;
    }
    return ION_OBJ(promise);
}


zend_object * ion_promisor_sequence_new(zval * init) {
    ion_promisor * sequence = ion_promisor_sequence_ex(0);

    if(ion_promisor_set_callbacks(ION_OBJ(sequence), init, NULL) == FAILURE) {
        zend_throw_exception(ion_ce_ION_RuntimeException, "Sequence expects a valid callbacks", 0);

    }
    return ION_OBJ(sequence);
}

zend_object * ion_promisor_deferred_new(zval * canceller) {
    ion_promisor * deferred = ion_promisor_deferred_ex(0);

    if(canceller) {
        ion_promisor_set_php_cb(&deferred->canceler, pion_cb_create_from_zval(canceller));
    }
    return ION_OBJ(deferred);
}


zend_object * ion_promisor_deferred_new_ex(promisor_action_t canceler) {
    ion_promisor * deferred = ion_promisor_deferred();

    if(canceler) {
        ion_promisor_set_internal_cb(&deferred->canceler, canceler);
    }
    return ION_OBJ(deferred);
}



int ion_promisor_set_callbacks(zend_object * promise_obj, zval * done, zval * fail) {
    ion_promisor * promise = get_object_instance(promise_obj, ion_promisor);
    if(done) {
        ion_promisor_set_php_cb(&promise->done, pion_cb_create_from_zval(done));
        if(!promise->done.cb.php) {
            return FAILURE;
        }
        promise->flags |= ION_PROMISOR_HAS_DONE;

        if(pion_cb_required_num_args(promise->done.cb.php) > 1) {
            return FAILURE;
        }
    }
    if(fail) {
        ion_promisor_set_php_cb(&promise->fail, pion_cb_create_from_zval(fail));
        if(!promise->fail.cb.php) {
            return FAILURE;
        }
        promise->flags |= ION_PROMISOR_HAS_FAIL;

        if(pion_cb_required_num_args(promise->fail.cb.php) > 1) {
            return FAILURE;
        }
    }
    return SUCCESS;
}

int ion_promisor_set_initial_callback(zend_object * sequence, zval * initial) {
    ion_promisor * promisor = get_object_instance(sequence, ion_promisor);
    if(initial) {
        ion_promisor_set_php_cb(&promisor->done, pion_cb_create_from_zval(initial));
        if(!promisor->done.cb.php) {
            return FAILURE;
        }
        promisor->flags |= ION_PROMISOR_HAS_DONE;

        if(pion_cb_required_num_args(promisor->done.cb.php) > 1) {
            return FAILURE;
        }
    }
    return SUCCESS;
}

zend_object * ion_promisor_push_callbacks(zend_object * promise_obj, zval * on_done, zval * on_fail) {
    ion_promisor * promisor = get_object_instance(promise_obj, ion_promisor);
    zend_object  * handler;

    if(promisor->flags & ION_PROMISOR_FINISHED) {
        handler = ion_promisor_promise_new(on_done, on_fail);
        ion_promisor_resolve(handler, &promisor->result, promisor->flags & ION_PROMISOR_FINISHED);
    } else {
        handler = ion_promisor_promise_new(on_done, on_fail);
        if(!handler) {
            return NULL;
        }
        PION_ARRAY_PUSH(promisor->handlers, promisor->handler_count, handler);
        zend_object_addref(handler);
    }
    ion_promisor_add_flags(handler, promisor->flags & ION_PROMISOR_NESTED_FLAGS);
    return handler;
}

void ion_promisor_cancel(zend_object * promisor_obj, const char *message) {
    zval           value;
    ion_promisor * promisor = get_object_instance(promisor_obj, ion_promisor);
    zend_object  * error = pion_exception_new(ion_class_entry(ION_Promise_CancelException), message, 0);

    zend_object_addref(promisor_obj);
    promisor->flags |= ION_PROMISOR_CANCELED;
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
        zend_throw_exception(ion_ce_ION_InvalidUsageException, "Trying to clone an internal promisor", 0);
        return clone_obj;
    }
    if(proto->await || proto->generator) {
        zend_throw_exception(ion_ce_ION_InvalidUsageException, "Promisor in progress", 0);
        return clone_obj;
    }
    if(proto->dtor) {
        clone->dtor = proto->dtor;
    }
    if(proto->done.type) {
        clone->done.type = proto->done.type;
        if(proto->done.type == ION_PROMISOR_CB_INTERNAL) {
            clone->done.cb.internal = proto->done.cb.internal;
        } else {
            clone->done.cb.php = pion_cb_dup(proto->done.cb.php);
        }
    }
    if(proto->fail.type) {
        clone->fail.type = proto->fail.type;
        if(proto->fail.type == ION_PROMISOR_CB_INTERNAL) {
            clone->fail.cb.internal = proto->fail.cb.internal;
        } else {
            clone->fail.cb.php = pion_cb_dup(proto->fail.cb.php);
        }
    }
    if(proto->handler_count) {
        ion_promisor * handler;
        ushort         extern_handlers = 0;
        clone->handler_count = 0;
        clone->handlers = emalloc(sizeof(zend_object *) * proto->handler_count);
        for(ushort i = 0; i<proto->handler_count; i++) {
            handler = get_object_instance(proto->handlers[i], ion_promisor);
            if((proto->flags & ION_PROMISOR_PROTOTYPE) && !(handler->flags & ION_PROMISOR_PROTOTYPE)) { // has external promisor
                clone->handlers[ clone->handler_count++ ] = proto->handlers[i];
                if(handler->await == proto_obj) {
                    handler->await = clone_obj;
                    obj_add_ref(clone_obj);
                    zend_object_release(proto_obj);
                }
                proto->handlers[i] = NULL;
                extern_handlers++;
            } else {
                if(handler->await != proto_obj) {
                    clone->handlers[ clone->handler_count++ ] = ion_promisor_clone(proto->handlers[i]);
                }
            }
        }
        if(clone->handler_count) {
            if(clone->handler_count != proto->handler_count) {
                clone->handlers = erealloc(clone->handlers, sizeof(zend_object *) * clone->handler_count);
            }
        } else {
            efree(clone->handlers);
            clone->handlers = NULL;
        }
        ion_promisor_cleanup(proto, extern_handlers);
        ion_promisor_autoclean(proto);
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
    if(promisor->name) {
        zend_string_release(promisor->name);
        promisor->name = NULL;
    }
    if(promisor->await) {
        zend_object_release(promisor->await);
        promisor->await = NULL;
    }
    if(promisor->generator) {
        zend_object_release(promisor->generator);
        promisor->generator = NULL;
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

int ion_promisor_append(zend_object * container, zend_object * handler) {
    ion_promisor * promisor = get_object_instance(container, ion_promisor);
    PION_ARRAY_PUSH(promisor->handlers, promisor->handler_count, handler);
    obj_add_ref(handler);
    return SUCCESS;
}

void ion_promisor_cleanup(ion_promisor * promisor, ushort removed) {
    if(removed) {
        if(promisor->handler_count - removed) {
            zend_object * *handlers = promisor->handlers;
            promisor->handlers = emalloc(sizeof(zend_object *) * (promisor->handler_count - removed));
            for(ushort i = 0, j = 0; i<promisor->handler_count; i++) {
                if(handlers[i]) {
                    promisor->handlers[j++] = handlers[i];
                }
            }
            promisor->handler_count = promisor->handler_count - removed;
            efree(handlers);
        } else {
            efree(promisor->handlers);
            promisor->handlers = NULL;
            promisor->handler_count = 0;
        }
    }
}

void ion_promisor_remove(zend_object * container, zend_object * handler) {
    ion_promisor * promisor = get_object_instance(container, ion_promisor);
    ushort         removed = 0;

    if(promisor->handler_count) {
        for(ushort i = 0; i < promisor->handler_count; i++) {
            if(promisor->handlers[i] == handler) {
                zend_object_release(promisor->handlers[i]);
                promisor->handlers[i] = NULL;
                removed++;
            }
        }

        ion_promisor_cleanup(promisor, removed);
    }
    ion_promisor_autoclean(promisor);
}

void ion_promisor_remove_named(zend_object * container, zend_string * name) {
    ion_promisor * promisor = get_object_instance(container, ion_promisor);
    ushort         removed = 0;

    if(promisor->handler_count) {
        for(ushort i = 0; i < promisor->handler_count; i++) {
            ion_promisor * handler = get_object_instance(promisor->handlers[i], ion_promisor);
            if(handler->name && strcmp(handler->name->val, name->val) == 0) {
                zend_object_release(promisor->handlers[i]);
                promisor->handlers[i] = NULL;
                removed++;
            }
        }

        ion_promisor_cleanup(promisor, removed);
    }
    ion_promisor_autoclean(promisor);
}

