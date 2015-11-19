#ifndef ION_ION_H
#define ION_ION_H

#include "pion.h"

typedef struct _ion_interval {
    struct timeval tv;
    zend_object  * promisor;
    zend_string  * name;
    ion_event    * timer;
} ion_interval;


#endif //ION_ION_H
