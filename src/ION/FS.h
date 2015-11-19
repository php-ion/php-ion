#ifndef ION_FS_H
#define ION_FS_H

#endif //ION_FS_H

// inotify: http://man7.org/linux/man-pages/man7/inotify.7.html
// kqueue: https://www.freebsd.org/cgi/man.cgi?query=kqueue&sektion=2
//         https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man2/kqueue.2.html#//apple_ref/c/func/kqueue
//         https://developer.apple.com/library/mac/documentation/Darwin/Conceptual/FSEvents_ProgGuide/KernelQueues/KernelQueues.html

typedef struct _ion_fs_watcher {
    zend_object * sequence;
    zend_string * pathname;
    int           flags;
    int           fd;
} ion_fs_watcher;

#if defined(HAVE_INOTIFY)
//...
#elif defined(HAVE_KQUEUE)
# define VNODE_EVENTS (NOTE_DELETE |  NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE)
# ifdef O_EVTONLY  // osx: descriptor requested for event notifications only
#  define OPEN_FLAGS O_EVTONLY
# else
#  define OPEN_FLAGS O_RDONLY
# endif
#endif

int ion_fs_watch_init();
int ion_fs_watch_close();
void ion_fs_watch_cb(evutil_socket_t fd, short what, void * arg);

ion_fs_watcher * ion_fs_watcher_add(const char * pathname, zend_long flags);
void ion_fs_watcher_remove(ion_fs_watcher * watcher);

void ion_fs_watcher_dtor(zend_object * sequence);
void ion_fs_watcher_clean(zval * zr);