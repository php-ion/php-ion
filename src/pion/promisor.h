#ifndef PION_PROMISOR_H
#define PION_PROMISOR_H

#include <php.h>
#include "init.h"
#include "../config.h"
#include "callback.h"
#include "exceptions.h"
#include <Zend/zend_generators.h>
#include <Zend/zend_closures.h>

// Result states
#define ION_PROMISOR_PROCESSING (1<<0)
#define ION_PROMISOR_DONE       (1<<1)
#define ION_PROMISOR_FAILED     (1<<2)
#define ION_PROMISOR_FINISHED   (ION_PROMISOR_DONE | ION_PROMISOR_FAILED)

// Info
#define ION_PROMISOR_PROGRESS  (1<<3)

// Fail reason
#define ION_PROMISOR_CANCELED  (1<<4)
#define ION_PROMISOR_TIMED_OUT (1<<5)

// Types
#define ION_PROMISOR_TYPE_PROMISE  (1<<6)
#define ION_PROMISOR_TYPE_SEQUENCE (1<<7)
#define ION_PROMISOR_TYPE_DEFERRED (1<<8)
#define ION_PROMISOR_INTERNAL      (1<<9)
#define ION_PROMISOR_PROTOTYPE     (1<<10)

// Callbacks flags
#define ION_PROMISOR_HAS_DONE        (1<<11)
#define ION_PROMISOR_HAS_FAIL        (1<<12)
#define ION_PROMISOR_HAS_PROGRESS    (1<<13)
#define ION_PROMISOR_YIELDED         (1<<14)

#define ION_PROMISOR_AUTOCLEAN       (1<<15)

#define ION_PROMISOR_NESTED_FLAGS  ION_PROMISOR_PROTOTYPE

extern ZEND_API zend_class_entry * ion_ce_ION_Promise;
extern ZEND_API zend_class_entry * ion_ce_ION_ResolvablePromise;
extern ZEND_API zend_class_entry * ion_ce_ION_Deferred;
extern ZEND_API zend_class_entry * ion_ce_ION_Sequence;
extern ZEND_API zend_class_entry * ion_ce_ION_Promise_CancelException;
extern ZEND_API zend_class_entry * ion_ce_ION_Promise_TimeoutException;

#define ion_ce_Generator zend_ce_generator
#define ion_ce_Closure   zend_ce_closure

typedef void (* promisor_canceler_t)(zend_object * promisor);
typedef void (* promisor_dtor_t)(zend_object * promisor);


typedef struct _ion_promisor {
    zend_object         std;
    zend_string       * name;
    zend_uint           flags;
    pion_cb           * done;
    pion_cb           * fail;
    zend_object       * await;
    zval                result;
    zend_object       * generator;
    void              * timer; // todo
    zend_object      ** handlers;
    ushort              handler_count;
    zend_class_entry  * scope;
    void              * object;
    promisor_canceler_t canceler;
    promisor_dtor_t     dtor;
} ion_promisor;

// Creating
zend_object * ion_promisor_promise_new(zval * done, zval * fail);
zend_object * ion_promisor_sequence_new(zval * init);
zend_object * ion_promisor_deferred_new(zval * cancelable);
zend_object * ion_promisor_deferred_new_ex(promisor_canceler_t canceler);

// Manipulations
zend_object * ion_promisor_clone(zend_object * proto_obj);
zend_object * ion_promisor_clone_obj(zval * zobject);

int ion_promisor_append(zend_object * container, zend_object * handler);
void ion_promisor_remove(zend_object * container, zend_object * handler);
void ion_promisor_remove_named(zend_object * container, zend_string * name);
void ion_promisor_cleanup(ion_promisor * promisor, ushort removed);  // realloc promisor->handlers, remove NULL elements
void ion_promisor_set_autoclean(zend_object * container, promisor_canceler_t autocleaner);

// Stores and destructors
#define ion_promisor_store(promisor, pobject)  get_object_instance(promisor, ion_promisor)->object = (void *) pobject
#define ion_promisor_store_get(promisor)       get_object_instance(promisor, ion_promisor)->object
#define ion_promisor_get_flags(promisor)       get_object_instance(promisor, ion_promisor)->flags
#define ion_promisor_add_flags(promisor, bits) get_object_instance(promisor, ion_promisor)->flags |= (bits)
#define ion_promisor_dtor(promisor, dtor_cb)   get_object_instance(promisor, ion_promisor)->dtor = dtor_cb

// Resolvers
void ion_promisor_resolve(zend_object * promisor, zval * result, int type);
void ion_promisor_cancel(zend_object * promisor, const char *message);
void ion_promisor_sequence_invoke(zend_object * promisor, zval * args);
// Notify
void ion_promisor_notify(zend_object * promisor, zval * info);

#define ion_promisor_done(promisor, result)  ion_promisor_resolve(promisor, result, ION_PROMISOR_DONE)
#define ion_promisor_fail(promisor, error)   ion_promisor_resolve(promisor, error, ION_PROMISOR_FAILED)

// Callbacks
int ion_promisor_set_callbacks(zend_object * promisor, zval * done, zval * fail);
zend_object * ion_promisor_push_callbacks(zend_object * promisor, zval * done, zval * fail);

// Instance

void ion_promisor_free(zend_object * promisor_obj);
zend_object * ion_promise_init(zend_class_entry * ce);
zend_object * ion_deferred_init(zend_class_entry * ce);
zend_object * ion_sequence_init(zend_class_entry * ce);

// Utils


#define PION_ARRAY_PUSH(array, counter, elem)                 \
    if(counter) {                                             \
        array = erealloc(array, sizeof(elem) * ++counter);    \
        array[counter - 1] = elem;                            \
    } else {                                                  \
        array = emalloc(sizeof(elem));                        \
        array[0] = elem;                                      \
        counter = 1;                                          \
    }

#define PION_ARRAY_POP(array, counter, elem)                   \
    if(counter) {                                              \
        elem = array[counter - 1];                             \
        if(counter == 1)  {                                    \
            efree(array);                                      \
            array = NULL;                                      \
            counter = 0;                                       \
        } else {                                               \
            array = erealloc(array, sizeof(elem) * --counter); \
        }                                                      \
    } else {                                                   \
        elem = NULL;                                           \
    }

#define Z_ISPROMISE_P(pz) Z_TYPE_P(pz) == IS_OBJECT && (  \
          Z_OBJCE_P(pz) == ion_class_entry(ION_Promise)   \
          || Z_OBJCE_P(pz) == ion_class_entry(ION_ResolvablePromise)  \
          || Z_OBJCE_P(pz) == ion_class_entry(ION_Deferred)   \
          || Z_OBJCE_P(pz) == ion_class_entry(ION_Sequence)   \
          || instanceof_function(Z_OBJCE_P(pz), ion_class_entry(ION_Promise)))

#define Z_ISPROMISE(zv) Z_ISPROMISE_P(&zv)

#define ion_promisor_autoclean(promisor)                    \
    if((promisor->flags & ION_PROMISOR_AUTOCLEAN) && !promisor->handler_count && promisor->canceler) {    \
        promisor->canceler(&promisor->std);     \
    }

#define ion_promisor_should_cancel(promisor) \
    ((promisor->flags & ION_PROMISOR_CANCEL_ON_EMPTY) && !promisor->handler_count && promisor->canceler)

#define ion_promisor_is_empty() (get_object_instance(object, ion_promisor)->handler_count == 0)

static zend_always_inline void ion_promisor_done_long(zend_object * promisor, long lval) {
    zval value;
    ZVAL_LONG(&value, lval);
    ion_promisor_done(promisor, &value);
}

static zend_always_inline void ion_promisor_done_true(zend_object * promisor) {
    zval value;
    ZVAL_TRUE(&value);
    ion_promisor_done(promisor, &value);
}

static zend_always_inline void ion_promisor_done_false(zend_object * promisor) {
    zval value;
    ZVAL_FALSE(&value);
    ion_promisor_done(promisor, &value);
}

static zend_always_inline void ion_promisor_done_null(zend_object * promisor) {
    zval value;
    ZVAL_NULL(&value);
    ion_promisor_done(promisor, &value);
}

static zend_always_inline void ion_promisor_done_object(zend_object * promisor, zend_object * object) {
    zval value;
    ZVAL_OBJ(&value, object);
    zval_add_ref(&value);
    ion_promisor_done(promisor, &value);
    zval_ptr_dtor(&value);
}

static zend_always_inline void ion_promisor_done_string(zend_object * promisor, zend_string * string, int dup) {
    zval value;
    if(dup) {
        ZVAL_STR_COPY(&value, string); // todo
    } else {
        ZVAL_STR(&value, string);
        zval_add_ref(&value);
    }
    ion_promisor_done(promisor, &value);
    zval_ptr_dtor(&value);
}

static zend_always_inline void ion_promisor_done_empty_string(zend_object * promisor) {
    zval value;
    zend_string * string = zend_string_init("", 0, 0);
    ZVAL_STR(&value, string);
    ion_promisor_done(promisor, &value);
    zval_ptr_dtor(&value);
}

static zend_always_inline void ion_promisor_exception(zend_object * promisor, zend_class_entry * ce, const char * message, long code) {
    zval value;
    zend_object * exception = pion_exception_new(ce, message, code);
    ZVAL_OBJ(&value, exception);
    ion_promisor_fail(promisor, &value);
    zval_ptr_dtor(&value);
}

static zend_always_inline void ion_promisor_exception_eg(zend_object * promisor, zend_class_entry * ce, const char * message, long code) {
    zval value;
    zend_object * exception;
    if(ce) {
        exception = pion_exception_new(ce, message, code);
        if(EG(exception)) {
            zend_exception_set_previous(EG(exception), exception);
            exception = EG(exception);
            EG(exception) = NULL;
        }
    } else if(EG(exception)) {
        exception = EG(exception);
        EG(exception) = NULL;
    } else {
        zend_error(E_ERROR, "invalid usage ion_promisor_exception_eg() function");
        return;
    }
    ZVAL_OBJ(&value, exception);
    ion_promisor_fail(promisor, &value);
    zval_ptr_dtor(&value);
}
//
//
//static zend_always_inline void ion_promisor_exception_ex(zend_object * promisor, zend_class_entry * ce, long code, const char * message, ...) {
//    va_list args;
//    zval value;
//    zend_object * exception = NULL;
//    va_start(args, message);
//    exception = pion_exception_new_ex(ce, code, message, args);
//    va_end(args);
//    ZVAL_OBJ(&value, exception);
//    ion_promisor_fail(promisor, &value);
//    zval_ptr_dtor(&value);
//}

#endif //PION_PROMISOR_H
