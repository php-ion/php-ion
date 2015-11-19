#include <sys/fcntl.h>
#include <sys/event.h>
#include "../pion.h"
#include "FS.h"

zend_class_entry * ion_ce_ION_FS;
zend_object_handlers ion_oh_ION_FS;

#if defined(HAVE_INOTIFY)

int ion_fs_watch_init() {
    GION(watch_fd) = inotify_init();
    if(ind < 0) {
        zend_error(E_ERROR, "FS watcher: Could not open inotify queue: %s", strerror(errno));
        return FAILURE;
    }
    notifier = event_new(ION(base), GION(watch_fd), EV_READ | EV_WRITE | EV_PERSIST | EV_ET, ion_fs_watch_cb, NULL);
    if(event_add(notifier, NULL) == FAILURE) {
        event_del(notifier);
        event_free(notifier);
        zend_error(E_ERROR, "FS watcher: Could not add listener for queue");
        return FAILURE;
    }
    GION(watch_event) = notifier;
    return SUCCESS;
}

int ion_fs_watch_close() {
    event_del(GION(watch_event));
    event_free(GION(watch_event));
    GION(watch_event) = NULL;
    close(GION(watch_fd));
    return SUCCESS;
}

ion_fs_watcher * ion_fs_watcher_add(const char * pathname, zend_long flags) {

    wd = inotify_add_watch( GION(watch_fd), "/tmp", IN_CREATE | IN_DELETE );
    int               fd;
    struct kevent     event;
    struct timespec   timeout = { 0, 0 };
    ion_fs_watcher  * watcher = NULL;

    watcher = ecalloc(1, sizeof(ion_fs_watcher));
    watcher->sequence = ion_promisor_sequence_new(NULL);
    watcher->fd = inotify_add_watch( fd, pathname, IN_CREATE | IN_DELETE );
    watcher->pathname = zend_string_init(pathname, strlen(pathname), 0);
    ion_promisor_store(watcher->sequence, watcher);
    ion_promisor_dtor(watcher->sequence, ion_fs_watcher_dtor);

    return watcher;
}

void ion_fs_watcher_remove(ion_fs_watcher * watcher) {
    zend_string_release(watcher->pathname);
    inotify_rm_watch(ind, watcher->fd);
    close(watcher->fd);
    efree(watcher);
}

void ion_fs_watch_cb(evutil_socket_t fd, short what, void * arg) {
    // todo need linux
}

#elif defined(HAVE_KQUEUE)

void ion_fs_watch_cb(evutil_socket_t fd, short what, void * arg) {
    struct timespec  timeout = { 0, 0 };
    struct kevent    event;
    int              has_event = 0;
    ion_fs_watcher * watcher = NULL;
    zval             result;

    while((has_event = kevent(GION(watch_fd), NULL, 0, &event, 1, &timeout))) {
        if ((has_event < 0) || (event.flags == EV_ERROR)) {
            /* An error occurred. */
            zend_error(E_ERROR, "FS watcher: An error occurred: %s", strerror(errno));
            return;
        }
        watcher = (ion_fs_watcher *)event.udata;
        ZVAL_STR(&result, watcher->pathname);
        ion_promisor_sequence_invoke(watcher->sequence, &result);
    }
}

int ion_fs_watch_init() {
    GION(watch_fd) = kqueue();
    ion_event * notifier = event_new(GION(base), GION(watch_fd), EV_READ | EV_WRITE | EV_PERSIST | EV_ET, ion_fs_watch_cb, NULL);
    if(event_add(notifier, NULL) == FAILURE) {
        event_del(notifier);
        event_free(notifier);
        zend_error(E_ERROR, "FS watcher: Could not open watchers queue: %s", strerror(errno));
        return FAILURE;
    }
    GION(watch_event) = notifier;
    return SUCCESS;
}

int ion_fs_watch_close() {
    event_del(GION(watch_event));
    event_free(GION(watch_event));
    GION(watch_event) = NULL;
    close(GION(watch_fd));
    return SUCCESS;
}

ion_fs_watcher * ion_fs_watcher_add(const char * pathname, zend_long flags) {
    int               fd;
    struct kevent     event;
    struct timespec   timeout = { 0, 0 };
    ion_fs_watcher  * watcher = NULL;

    fd = open(pathname, OPEN_FLAGS);
    if (fd <= 0) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0 , "Failed to open file %s for events: %s", pathname, strerror(errno));
        return NULL;
    }

    watcher = ecalloc(1, sizeof(ion_fs_watcher));
    watcher->sequence = ion_promisor_sequence_new(NULL);
    watcher->fd = fd;
    watcher->pathname = zend_string_init(pathname, strlen(pathname), 0);
    ion_promisor_store(watcher->sequence, watcher);
    ion_promisor_dtor(watcher->sequence, ion_fs_watcher_dtor);

    memset(&event, 0, sizeof(event));
    EV_SET(&event, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, VNODE_EVENTS, 0, watcher);
    if (kevent(GION(watch_fd), &event, 1, NULL, 0, &timeout) == -1) {
        zend_object_release(watcher->sequence);
        zend_throw_exception(ion_class_entry(ION_RuntimeException), "Failed to add fsnotify event", 0);
        return NULL;
    }

    return watcher;
}

void ion_fs_watcher_remove(ion_fs_watcher * watcher) {
    struct kevent event;
    struct timespec timeout = { 0, 0 };
    EV_SET(&event, watcher->fd, EVFILT_VNODE, EV_DELETE, 0, 0, NULL);
    if(kevent(GION(watch_fd), &event, 1, NULL, 0, &timeout) == -1) {
        zend_error(E_NOTICE, "FS watcher: Could not remove watcher from queue: %s", strerror(errno));
    }
    zend_string_release(watcher->pathname);
    close(watcher->fd);
    efree(watcher);
}

#endif

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
    zend_object  * deferred = (zend_object *) ctx;

    if(what & BEV_EVENT_EOF) {
        size_t size = evbuffer_get_length( bufferevent_get_input(two) );
        zend_string  * data = zend_string_alloc(size - 1, 0);
        bufferevent_read(two, ZSTR_VAL(data), size);
        ion_promisor_done_string(deferred, data, 0);
        zend_string_release(data);
    } else if(what &  BEV_EVENT_ERROR) {
        ion_promisor_exception(deferred, ion_class_entry(ION_RuntimeException), "Failed to read file: buffer is unreachble", 0);
    }
    bufferevent_disable(two, EV_READ);
    zend_object_release(deferred);

}

void ion_fs_read_file_dtor(zend_object * deferred) {
    ion_buffer   * two = ion_promisor_store_get(deferred);
    bufferevent_free(two);
}

/** public static function ION\FS::readFile(string $filename, int $offset = 0, int $length = -1) : Deferred */
CLASS_METHOD(ION_FS, readFile) {
    zend_object     * deferred     = NULL;
    zend_string     * filename     = NULL;
    zend_long         offset       = 0;
    zend_long         length       = -1;
    int               fd;
    struct stat       st;
    ion_evbuffer    * evbuffer;
    evutil_socket_t pair[2] = {-1, -1};


    ZEND_PARSE_PARAMETERS_START(1,3)
        Z_PARAM_STR(filename)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(offset)
        Z_PARAM_LONG(length)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    fd = open(filename->val, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
    if(fd == -1) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to open file %s: %s", filename->val, strerror(errno));
        return;
    }
    if (fstat(fd, &st) == FAILURE) {
        close(fd);
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Stat failed for %s: %s", filename->val, strerror(errno));
        return;
    }
    if(length < 0 || length > st.st_size) {
        length = st.st_size;
    }

    if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1) {
        zend_throw_exception(ion_class_entry(ION_RuntimeException), "Failed to create pair stream", 0);
        return;
    }
    ion_buffer * one = bufferevent_socket_new(GION(base), pair[0], STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE);
    ion_buffer * two = bufferevent_socket_new(GION(base), pair[1], STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE);
    bufferevent_enable(one, EV_WRITE);
    bufferevent_enable(two, EV_READ);
//    bufferevent_setwatermark(two, EV_READ, (size_t)length + 1, (size_t)length + 10);
    deferred = ion_promisor_deferred_new_ex(zend_object_release);
    ion_promisor_store(deferred, two);
    ion_promisor_dtor(deferred, ion_fs_read_file_dtor);

    evbuffer = bufferevent_get_output(one);
    evbuffer_set_flags(evbuffer, EVBUFFER_FLAG_DRAINS_TO_FD);
    if(evbuffer_add_file(evbuffer, fd, (ev_off_t)offset, (ev_off_t)length) == FAILURE) {
        close(fd);
        ion_promisor_free(deferred);
        zend_throw_exception(ion_class_entry(ION_RuntimeException), "Failed to transfer data via sendfile/mmap", 0);
        return;
    }
    bufferevent_write(one, "\0", 1);
    bufferevent_setcb(one, NULL, ion_fs_file_sent_one, ion_fs_pair_close_one, NULL);
    bufferevent_setcb(two, NULL, NULL, ion_fs_pair_close_two, deferred);
    obj_add_ref(deferred);
    RETURN_OBJ(deferred);
}

METHOD_ARGS_BEGIN(ION_FS, readFile, 1)
    METHOD_ARG_STRING(filename, 0)
    METHOD_ARG_LONG(offset, 0)
    METHOD_ARG_LONG(length, 0)
METHOD_ARGS_END()

void ion_fs_watcher_dtor(zend_object * sequence) {
    ion_fs_watcher * watcher = ion_promisor_store_get(sequence);
    if(watcher) {
        ion_fs_watcher_remove(watcher);
    }
}

void ion_fs_watcher_clean(zval * zr) {
    ion_fs_watcher * watcher = Z_PTR_P(zr);
    zend_object_release(watcher->sequence);
}

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
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "File or directory %s doesn't exists", filename->val);
        return;
    }

    watcher = zend_hash_str_find_ptr(GION(watchers), realpath, strlen(realpath));
    if(watcher) {
        obj_add_ref(watcher->sequence);
        RETURN_OBJ(watcher->sequence);
    }

    watcher = ion_fs_watcher_add(realpath, flags);
    if(!watcher) {
        return; // exception has been thrown
    }
    if(!zend_hash_str_add_ptr(GION(watchers), realpath, strlen(realpath), watcher)) {
        zend_object_release(watcher->sequence);
        zend_throw_exception(ion_class_entry(ION_RuntimeException), "Failed to store watcher", 0);
        return;
    }
    obj_add_ref(watcher->sequence);
    RETURN_OBJ(watcher->sequence);
}

METHOD_ARGS_BEGIN(ION_FS, watch, 1)
    METHOD_ARG_STRING(filename, 0)
    METHOD_ARG_LONG(events, 0)
METHOD_ARGS_END()

CLASS_METHOD(ION_FS, unwatchAll) {
    zend_hash_clean(GION(watchers));
}

METHOD_WITHOUT_ARGS(ION_FS, unwatchAll);

CLASS_METHODS_START(ION_FS)
    METHOD(ION_FS, readFile,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_FS, watch,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_FS, unwatchAll, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_FS) {
    PION_REGISTER_STATIC_CLASS(ION_FS, "ION\\FS");
    PION_CLASS_CONST_LONG(ION_FS, "WATCH_MODIFY", 1);

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