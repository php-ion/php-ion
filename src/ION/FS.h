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