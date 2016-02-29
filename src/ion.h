
#ifndef ION_FRAMEWORK_H
#define ION_FRAMEWORK_H

#include "ion/strings.h"
#include "ion/init.h"
#include "ion/errors.h"
#include "ion/memory.h"
#include "ion/exceptions.h"
#include "ion/debug.h"
#include "ion/callback.h"
#include "ion/deferred_queue.h"
#include "ion/engine.h"
#include "ion/promisor.h"
#include "ion/net.h"
#include "ion/dns.h"
#include "ion/timers.h"
#include "ion/stream.h"
#include "ion/crypto.h"
#include "ion/process.h"
#include "ion/fs.h"
#include "ion/http.h"

#define ION_LOOP_STARTED   1
#define ION_LOOP_CORRUPTED 2

#define ION_MAX_PRIORITY   6
#define ION_PRIORITY_DEFAULT   3

#define has_one_bit(b) (b && !(b & (b-1)))

#endif //ION_FRAMEWORK_H
