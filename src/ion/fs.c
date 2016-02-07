#include "init.h"
#include "promisor.h"
#include "fs.h"


#if defined(HAVE_INOTIFY)

int ion_fs_watch_init() {
    return SUCCESS;
}

int ion_fs_watch_close() {

    return SUCCESS;
}

ion_fs_watcher * ion_fs_watcher_add(const char * pathname, zend_long flags) {
    int               fd;
    ion_fs_watcher  * watcher = NULL;

    fd = inotify_init();
    if(fd < 0) {
        zend_throw_exception_ex(ion_ce_ION_RuntimeException, 0 , "Failed to init inotify for file %s: %s", pathname, strerror(errno));
        return NULL;
    }
    watcher = ecalloc(1, sizeof(ion_fs_watcher));
    watcher->sequence = ion_promisor_sequence_new(NULL);
    watcher->fd = fd;
    watcher->pathname = zend_string_init(pathname, strlen(pathname), 0);
    ion_promisor_store(watcher->sequence, watcher);
    ion_promisor_dtor(watcher->sequence, ion_fs_watcher_dtor);
    watcher->event = event_new(GION(base), fd, EV_READ | EV_WRITE | EV_PERSIST | EV_ET, ion_fs_watch_cb, watcher);
    if(event_add(watcher->event, NULL) == FAILURE) {
        zend_object_release(watcher->sequence);
        zend_throw_exception_ex(ion_ce_ION_RuntimeException, 0 , "Could not activate listener of %s", pathname);
        return NULL;
    }
    if(inotify_add_watch(fd, pathname, ION_FS_WATCH_EVENTS) < 0) {
        zend_object_release(watcher->sequence);
        zend_throw_exception_ex(ion_ce_ION_RuntimeException, 0 , "Failed to listen of %s", pathname);
        return NULL;
    }

    return watcher;

}

void ion_fs_watcher_remove(ion_fs_watcher * watcher) {
    zend_string_release(watcher->pathname);
    close(watcher->fd);
    event_del(watcher->event);
    event_free(watcher->event);
    efree(watcher);
}

void ion_fs_watch_cb(evutil_socket_t fd, short what, void * arg) {
    ssize_t          read_bytes = 0;
    ssize_t          has_bytes = 0;
    ion_fs_watcher * watcher = (ion_fs_watcher *)arg;
    zval             result;
    struct inotify_event event;
    char             path[MAXPATHLEN];

    array_init(&result);
    while(ioctl(fd, FIONREAD, &has_bytes) != FAILURE && has_bytes) {
        read_bytes = read(watcher->fd, (void *)&event, INOTIFY_EVENT_SIZE);
        if(read_bytes < 0) {
            /* An error occurred. */
            zend_error(E_ERROR, "FS watcher: An error occurred: %s", strerror(errno));
            return;
        }
        if(event.len) {
            memcpy(path, watcher->pathname->val, watcher->pathname->len);
            memcpy(path + watcher->pathname->len, "/", 1);
            read(watcher->fd, (void *)path + watcher->pathname->len + 1, event.len);
            add_assoc_long(&result, path, event.mask);
        } else {
            add_assoc_long(&result, watcher->pathname->val, event.mask);
        }
    }
    ion_promisor_sequence_invoke(watcher->sequence, &result);
    zval_ptr_dtor(&result);
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
        array_init(&result);
        add_assoc_long(&result, watcher->pathname->val, event.fflags);
        ion_promisor_sequence_invoke(watcher->sequence, &result);
        zval_ptr_dtor(&result);
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
    EV_SET(&event, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, ION_FS_WATCH_EVENTS, 0, watcher);
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

void ion_fs_watcher_dtor(ion_promisor * sequence) {
    ion_fs_watcher * watcher = sequence->object;
    if(watcher) {
        ion_fs_watcher_remove(watcher);
    }
}

void ion_fs_watcher_clean(zval * zr) {
    ion_fs_watcher * watcher = Z_PTR_P(zr);
    zend_object_release(watcher->sequence);
}