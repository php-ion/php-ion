#ifndef PION_PROMISOR_H
#define PION_PROMISOR_H

//#include <php.h>
#include "ion_init.h"
//#include "../config.h"
#include "ion_callback.h"
#include "ion_exceptions.h"
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
#define ION_PROMISOR_SUSPENDED       (1<<14)
#define ION_PROMISOR_MULTI_ARGS      (1<<15)

#define ION_PROMISOR_AUTOCLEAN       (1<<16)

#define ION_PROMISOR_NESTED_FLAGS  ION_PROMISOR_PROTOTYPE
#define ION_PROMISOR_ARGS_NUM_SHIFT  24

extern ZEND_API zend_class_entry * ion_ce_ION_Promise;
extern ZEND_API zend_class_entry * ion_ce_ION_ResolvablePromise;
extern ZEND_API zend_class_entry * ion_ce_ION_Deferred;
extern ZEND_API zend_class_entry * ion_ce_ION_Sequence;
extern ZEND_API zend_class_entry * ion_ce_ION_Promise_CancelException;
//extern ZEND_API zend_class_entry * ion_ce_ION_Promise_TimeoutException;

#define ion_ce_Generator zend_ce_generator
#define ion_ce_Closure   zend_ce_closure

typedef void (* promisor_dtor_t)(zval * data);
typedef zval (* promisor_action_t)(ion_promisor * promisor, zval * data);

enum ion_promisor_cb_type {
    ION_PROMISOR_CB_UNSET,
    ION_PROMISOR_CB_PHP,
    ION_PROMISOR_CB_INTERNAL,
};


struct _ion_promisor_action_cb {
    zend_uchar type;
    union {
        pion_cb * php;
        promisor_action_t internal;
    } cb;
};

struct _ion_promisor {
    uint32_t            flags;

    ion_promisor_action_cb done;     // done callback for promises or init callback for sequences
    ion_promisor_action_cb fail;     // fail callback for promises
    ion_promisor_action_cb canceler; // cancel callback for deterred and clean callback for sequences

    ion_promisor      * await;     // yield-promisor
    zval                result;
    zend_object       * generator; // currently running generator
    ion_promisor     ** handlers;  // next promisors (then)
    ushort              handler_count; // next promisors count
    zend_class_entry  * scope;     //
    zval                object;    // if IS_PTR function dtor will be used in destructor
//    void              * object;
    promisor_dtor_t     dtor;      // internal custom destructor
    zend_string       * name;

    zend_object         php_object;
};

// Creating
ion_promisor * ion_promisor_promise_new(zval * done, zval * fail);
ion_promisor * ion_promisor_sequence_new(zval * init);
ion_promisor * ion_promisor_deferred_new(zval * cancelable);
ion_promisor * ion_promisor_deferred_new_ex(promisor_action_t canceler);

// Manipulations
ion_promisor * ion_promisor_clone(ion_promisor * proto);
zend_object * ion_promisor_zend_clone(zval * zobject);

int ion_promisor_append(ion_promisor * container, ion_promisor * handler);
void ion_promisor_remove(ion_promisor * container, ion_promisor * handler);
void ion_promisor_remove_named(ion_promisor * container, zend_string * name);
void ion_promisor_cleanup(ion_promisor * promisor, ushort removed);  // realloc promisor->handlers, remove NULL elements


static zend_always_inline ion_promisor * ion_promisor_new(zend_class_entry * ce, uint32_t flags) {
    zval object;
    ion_promisor * promisor;

    object_init_ex(&object, ce);
    promisor = ION_ZVAL_OBJECT(object, ion_promisor);
    promisor->flags |= flags;
    return promisor;
}

static zend_always_inline void ion_promisor_set_php_cb(ion_promisor_action_cb * pcb, pion_cb * cb) {
    if(pcb->type == ION_PROMISOR_CB_PHP) {
        pion_cb_free(pcb->cb.php);
        pcb->type = ION_PROMISOR_CB_UNSET;
        pcb->cb.php = NULL;
    }
    if(cb) {
        pcb->cb.php = cb;
        pcb->type = ION_PROMISOR_CB_PHP;
    } else {
        pcb->type = ION_PROMISOR_CB_UNSET;
        pcb->cb.php = NULL;
    }
}

static zend_always_inline void ion_promisor_set_internal_cb(ion_promisor_action_cb * pcb, promisor_action_t cb) {
    if(pcb->type == ION_PROMISOR_CB_PHP) {
        pion_cb_free(pcb->cb.php);
    }
    pcb->type = ION_PROMISOR_CB_INTERNAL;
    pcb->cb.internal = cb;
}

static zend_always_inline void obj_promisor_set_dtor(zend_object * promisor, promisor_dtor_t dtor) {
    ION_ZOBJ_OBJECT(promisor, ion_promisor)->dtor = dtor;
}

#define ion_promisor_set_autoclean(promisor, cb) \
    ion_promisor_set_internal_cb(&ION_ZOBJ_OBJECT(promisor, ion_promisor)->canceler, cb)

#define ion_promisor_set_internal_done(promisor, cb) \
    ion_promisor_set_internal_cb(&(promisor->done), cb)

// Resolvers
void ion_promisor_resolve(ion_promisor * promise, zval * result, uint32_t type);
void ion_promisor_cancel(ion_promisor * promise, const char * message);

void ion_promisor_done_long(ion_promisor * promisor, long lval);
void ion_promisor_done_true(ion_promisor * promisor);
void ion_promisor_done_false(ion_promisor * promisor);
void ion_promisor_done_null(ion_promisor * promisor);
void ion_promisor_done_object(ion_promisor * promisor, zend_object * object);
void ion_promisor_done_string(ion_promisor * promisor, zend_string * string, int dup);
void ion_promisor_done_empty_string(ion_promisor * promisor);
void ion_promisor_throw(ion_promisor * promisor, zend_class_entry * ce, const char * message, long code);
void ion_promisor_rethrow(ion_promisor * promisor, zend_class_entry * ce, const char * message, long code);

ion_promisor * ion_promisor_done_long_ex(ion_promisor * promisor, long lval);
ion_promisor * ion_promisor_done_true_ex(ion_promisor * promisor);
ion_promisor * ion_promisor_done_false_ex(ion_promisor * promisor);
ion_promisor * ion_promisor_done_null_ex(ion_promisor * promisor);
ion_promisor * ion_promisor_done_object_ex(ion_promisor * promisor, zend_object * object);
ion_promisor * ion_promisor_done_string_ex(ion_promisor * promisor, zend_string * string, int dup);
ion_promisor * ion_promisor_done_empty_string_ex(ion_promisor * promisor);

void ion_promisor_done(ion_promisor * promisor, zval * result);
ion_promisor * ion_promisor_done_ex(ion_promisor * promisor, zval * result);
ion_promisor * ion_promisor_done_args(ion_promisor * promisor, zval * args, int count);
void ion_promisor_fail(ion_promisor * promisor, zval * exception);
ion_promisor * ion_promisor_fail_ex(ion_promisor * promisor, zval * exception);

// Callbacks
int ion_promisor_set_callbacks(ion_promisor * promise, zval * done, zval * fail);
int ion_promisor_set_initial_callback(ion_promisor * promise, zval * initial);
ion_promisor * ion_promisor_push_callbacks(ion_promisor * promise, zval * done, zval * fail);

// Object
void ion_promisor_set_object(ion_promisor * promisor, zval * object, int ref_delta);
void ion_promisor_set_object_zobj(ion_promisor * promisor, zend_object * object, int ref_delta);
void ion_promisor_set_object_ptr(ion_promisor * promisor, void * object, promisor_dtor_t object_dtor);
void ion_promisor_remove_object(ion_promisor * promisor);

// Instance

void ion_promisor_zend_free(zend_object * promisor_obj);
zend_object * ion_promise_zend_init(zend_class_entry * ce);
zend_object * ion_deferred_zend_init(zend_class_entry * ce);
zend_object * ion_sequence_zend_init(zend_class_entry * ce);

// Utils
#define Z_ISPROMISE_P(pz) Z_TYPE_P(pz) == IS_OBJECT && (    \
          Z_OBJCE_P(pz) == ion_ce_ION_Promise               \
          || Z_OBJCE_P(pz) == ion_ce_ION_ResolvablePromise  \
          || Z_OBJCE_P(pz) == ion_ce_ION_Deferred           \
          || Z_OBJCE_P(pz) == ion_ce_ION_Sequence           \
          || instanceof_function(Z_OBJCE_P(pz), ion_ce_ION_Promise))

#define Z_ISPROMISE(zv) Z_ISPROMISE_P(&zv)

static zend_always_inline void ion_promisor_try_clean(ion_promisor * promisor) {
    if((promisor->flags & ION_PROMISOR_AUTOCLEAN) && !promisor->handler_count && promisor->canceler.type) {
        if(promisor->canceler.type == ION_PROMISOR_CB_INTERNAL) {
            promisor->canceler.cb.internal(promisor, NULL);
        } else {
            zend_object_addref(ION_OBJECT_ZOBJ(promisor));
            pion_cb_void_without_args(promisor->canceler.cb.php);
            zend_object_release(ION_OBJECT_ZOBJ(promisor));
        }
    }
}

#define ion_promisor_autoclean(promisor) ion_promisor_try_clean(promisor)

#endif //PION_PROMISOR_H
