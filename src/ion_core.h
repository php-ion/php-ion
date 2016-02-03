
#ifndef ION_FRAMEWORK_H
#define ION_FRAMEWORK_H

#include "core/strings.h"
#include "core/init.h"
#include "core/errors.h"
#include "core/memory.h"
#include "core/exceptions.h"
#include "core/debug.h"
#include "core/callback.h"
#include "core/engine.h"
#include "core/promisor.h"
#include "core/net.h"
#include "core/crypto.h"
#include "core/stream.h"
#include "core/http.h"

#define ION_LOOP_STARTED   1
#define ION_LOOP_CORRUPTED 2

#define ION_MAX_PRIORITY   6
#define ION_PRIORITY_DEFAULT   3

#define has_one_bit(b) (b && !(b & (b-1)))

#endif //ION_FRAMEWORK_H
