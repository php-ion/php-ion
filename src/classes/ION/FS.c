#include "ion.h"

zend_class_entry * ion_ce_ION_FS;
zend_class_entry * ion_ce_ION_FSException;

void ion_fs_file_sent_one(ion_buffer * one, void * ctx) {
    bufferevent_disable(one, EV_READ | EV_WRITE);
    bufferevent_free(one);
}

void ion_fs_pair_close_one(ion_buffer * one, short what, void * ctx) {
    if(what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_disable(one, EV_READ | EV_WRITE);
        bufferevent_free(one);
    }
}

void ion_fs_pair_close_two(ion_buffer * two, short what, void * ctx) {
    ion_promisor  * deferred = ctx;

    if(what & BEV_EVENT_EOF) {
        size_t size = evbuffer_get_length( bufferevent_get_input(two) );
        zend_string  * data = zend_string_alloc(size, 0);
        bufferevent_read(two, ZSTR_VAL(data), size);
        ZSTR_VAL(data)[size] = '\0';
        ion_promisor_done_string(deferred, data, 0);
        zend_string_release(data);
    } else if(what &  BEV_EVENT_ERROR) {
        ion_promisor_throw(deferred, ion_ce_ION_RuntimeException, ERR_ION_FS_READ_BROKEN_PIPE, 0);
    }
    bufferevent_disable(two, EV_READ);
    ion_object_release(deferred);

}

void ion_fs_read_file_dtor(zval * ptr) {
    ion_buffer   * two = Z_PTR_P(ptr);
    bufferevent_free(two);
}

/** public static function ION\FS::readFile(string $filename, int $offset = 0, int $length = -1) : Deferred */
CLASS_METHOD(ION_FS, readFile) {
    ion_promisor    * deferred     = NULL;
    zend_string     * filename     = NULL;
    zend_long         offset       = 0;
    zend_long         length       = -1;
    int               fd;
    struct stat       st;
    ion_evbuffer    * evbuffer;
    evutil_socket_t   pair[2] = {-1, -1};

    ZEND_PARSE_PARAMETERS_START(1,3)
        Z_PARAM_STR(filename)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(offset)
        Z_PARAM_LONG(length)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    fd = open(filename->val, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
    if(fd == -1) {
        zend_throw_exception_ex(ion_ce_ION_FSException, 0, ERR_ION_FS_READ_CANT_OPEN, filename->val, strerror(errno));
        return;
    }
    if (fstat(fd, &st) == FAILURE) {
        close(fd);
        zend_throw_exception_ex(ion_ce_ION_FSException, 0, ERR_ION_FS_READ_CANT_STAT, filename->val, strerror(errno));
        return;
    }
    if(length < 0 || length > st.st_size) {
        length = st.st_size;
    }

    if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1) {
        zend_throw_exception(ion_ce_ION_FSException, ERR_ION_FS_READ_CANT_OPEN_PIPE, 0);
        return;
    }
    ion_buffer * one = bufferevent_socket_new(GION(base), pair[0], STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE);
    ion_buffer * two = bufferevent_socket_new(GION(base), pair[1], STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE);
    bufferevent_enable(one, EV_WRITE);
    bufferevent_enable(two, EV_READ);
//    bufferevent_setwatermark(two, EV_READ, (size_t)length + 1, (size_t)length + 10);
    deferred = ion_promisor_deferred_new_ex(NULL);

    evbuffer = bufferevent_get_output(one);
    evbuffer_set_flags(evbuffer, EVBUFFER_FLAG_DRAINS_TO_FD);
    if(evbuffer_add_file(evbuffer, fd, (ev_off_t)offset, (ev_off_t)length) == FAILURE) {
        close(fd);
        ion_object_release(deferred);
        zend_throw_exception(ion_ce_ION_FSException, ERR_ION_FS_READ_SENDFILE_FAILED, 0);
        return;
    }
    ion_promisor_set_object_ptr(deferred, two, ion_fs_read_file_dtor);
//    bufferevent_write(one, "\0", 1);
    bufferevent_setcb(one, NULL, ion_fs_file_sent_one, ion_fs_pair_close_one, NULL);
    bufferevent_setcb(two, NULL, NULL, ion_fs_pair_close_two, deferred);
    ion_object_addref(deferred);
    RETURN_ION_OBJ(deferred);
}

METHOD_ARGS_BEGIN(ION_FS, readFile, 1)
    ARGUMENT(filename, IS_STRING)
    ARGUMENT(offset, IS_LONG)
    ARGUMENT(length, IS_LONG)
METHOD_ARGS_END()

/** public static function ION\FS::watch(string $filename, int $events = 0) : Sequence */
CLASS_METHOD(ION_FS, watch) {
    zend_string     * filename = NULL;
    zend_long         flags    = 0;
    char              realpath[MAXPATHLEN];
    ion_fs_watcher  * watcher = NULL;

    ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_STR(filename)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(!VCWD_REALPATH(filename->val, realpath)) {
        zend_throw_exception_ex(ion_ce_ION_FSException, 0, ERR_ION_FS_EVENT_FILE_NOT_FOUND, filename->val);
        return;
    }

    watcher = zend_hash_str_find_ptr(GION(watchers), realpath, strlen(realpath));
    if(watcher) {
        ion_object_addref(watcher->sequence);
        RETURN_ION_OBJ(watcher->sequence);
    }

    watcher = ion_fs_watcher_add(realpath, flags);
    if(!watcher) {
        return; // exception has been thrown
    }
    if(!zend_hash_str_add_ptr(GION(watchers), realpath, strlen(realpath), watcher)) {
        ion_object_release(watcher->sequence);
        zend_throw_exception(ion_ce_ION_FSException, ERR_ION_FS_EVENT_CANT_STORE_WATCHER, 0);
        return;
    }
    ion_object_addref(watcher->sequence);
    RETURN_ION_OBJ(watcher->sequence);
}

METHOD_ARGS_BEGIN(ION_FS, watch, 1)
    ARGUMENT(filename, IS_STRING)
    ARGUMENT(events, IS_LONG)
METHOD_ARGS_END()

CLASS_METHOD(ION_FS, unwatchAll) {
    zend_hash_clean(GION(watchers));
}

METHOD_WITHOUT_ARGS(ION_FS, unwatchAll);

METHODS_START(methods_ION_FS)
    METHOD(ION_FS, readFile,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_FS, watch,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_FS, unwatchAll, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_FS) {
    ion_register_static_class(ion_ce_ION_FS, "ION\\FS", methods_ION_FS);
    ion_register_exception(ion_ce_ION_FSException, ion_ce_ION_RuntimeException, "ION\\FSException");

    ion_class_declare_constant_long(ion_ce_ION_FS, "WATCH_MODIFY", 1);

    return ion_fs_watch_init();
}

PHP_RINIT_FUNCTION(ION_FS) {
    ALLOC_HASHTABLE(GION(watchers));
    zend_hash_init(GION(watchers), 128, NULL, ion_fs_watcher_clean, 0);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_FS) {
    zend_hash_clean(GION(watchers));
    zend_hash_destroy(GION(watchers));
    FREE_HASHTABLE(GION(watchers));
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_FS) {
    return ion_fs_watch_close();
}