#ifndef ION_ION_H
#define ION_ION_H

#include "pion.h"

#define PRESERVE_TIMERS  1
#define PRESERVE_SIGNALS 2
#define PRESERVE_FLAGS   3
#define RECREATE_BASE    4

typedef struct _ion_interval {
    struct timeval tv;
    zend_object  * promisor;
    zend_string  * name;
    event        * timer;
} ion_interval;


#endif //ION_ION_H
