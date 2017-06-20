
#ifndef ION_FRAMEWORK_H
#define ION_FRAMEWORK_H

#include "ion/ion_strings.h"
#include "ion/ion_init.h"
#include "ion/ion_errors.h"
#include "ion/ion_memory.h"
#include "ion/ion_exceptions.h"
#include "ion/ion_debug.h"
#include "ion/ion_callback.h"
#include "ion/ion_deferred_queue.h"
#include "ion/ion_zend.h"
#include "ion/ion_promisor.h"
#include "ion/ion_net.h"
#include "ion/ion_dns.h"
#include "ion/ion_timers.h"
#include "ion/ion_stream.h"
#include "ion/ion_crypto.h"
#include "ion/ion_process.h"
#include "ion/ion_fs.h"
#include "ion/ion_http.h"

#define ION_LOOP_STARTED   1
#define ION_LOOP_CORRUPTED 2

#define ION_MAX_PRIORITY   6
#define ION_PRIORITY_DEFAULT   3

#define has_one_bit(b) (b && !(b & (b-1)))

#endif //ION_FRAMEWORK_H
