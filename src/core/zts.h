#ifndef PION_ZTS_H
#define PION_ZTS_H

#ifdef ZTS
#include "TSRM.h"
#endif

#if defined(COMPILE_DL_ION) && defined(ZTS)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

#endif //PION_ZTS_H
