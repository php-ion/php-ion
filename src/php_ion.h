//
// Created by Ivan Shalganov on 07.06.15.
//

#ifndef ION_PHP_ION_H
#define ION_PHP_ION_H

#include <php.h>

extern zend_module_entry ion_module_entry;
#define phpext_ion_ptr &ion_module_entry

#define PHP_ION_VERSION_NUMBER 200
#define PHP_ION_VERSION "0.2.0"

PHP_MINIT_FUNCTION(ion);

PHP_MINFO_FUNCTION(ion);
PHP_RINIT_FUNCTION(ion);

PHP_RSHUTDOWN_FUNCTION(ion);

#define STARTUP_MODULE(module)   \
    ZEND_MODULE_STARTUP_N(module)(INIT_FUNC_ARGS_PASSTHRU)

#define SHUTDOWN_MODULE(module)   \
    ZEND_MODULE_SHUTDOWN_N(module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)

#define ACTIVATE_MODULE(module)   \
    ZEND_MODULE_ACTIVATE_N(module)(INIT_FUNC_ARGS_PASSTHRU)

#define DEACTIVATE_MODULE(module)   \
    ZEND_MODULE_DEACTIVATE_N(module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)

#endif //ION_PHP_ION_H
