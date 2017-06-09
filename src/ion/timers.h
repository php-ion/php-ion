#ifndef ION_TIMERS_H
#define ION_TIMERS_H

typedef struct _ion_interval {
    struct timeval tv;
    zend_bool      repeat;
    ion_promisor * promisor;
    zend_string  * name;
    ion_event    * timer;
} ion_interval;



#endif //ION_TIMERS_H
