#ifndef ION_PHP_ION_H
#define ION_PHP_ION_H

#include <php.h>

extern zend_module_entry ion_module_entry;
#define phpext_ion_ptr &ion_module_entry

PHP_MINIT_FUNCTION(exceptions);

PHP_MINIT_FUNCTION(ion);
PHP_MSHUTDOWN_FUNCTION(ion);

PHP_MINIT_FUNCTION(ION_Debug);

PHP_MINIT_FUNCTION(ION_Data_LinkedList);
PHP_MSHUTDOWN_FUNCTION(ION_Data_LinkedList);

PHP_MINIT_FUNCTION(ION_Data_SkipList);
PHP_MSHUTDOWN_FUNCTION(ION_Data_SkipList);

PHP_MINIT_FUNCTION(promisor);
PHP_RINIT_FUNCTION(promisor);
PHP_RSHUTDOWN_FUNCTION(promisor);
PHP_MSHUTDOWN_FUNCTION(promisor);

PHP_MINIT_FUNCTION(ION_Promise);

PHP_MINIT_FUNCTION(ION_ResolvablePromise);

PHP_MINIT_FUNCTION(ION_Deferred);
PHP_MSHUTDOWN_FUNCTION(ION_Deferred);

PHP_MINIT_FUNCTION(ION_Sequence);

PHP_MINIT_FUNCTION(ION_Crypto);
PHP_MSHUTDOWN_FUNCTION(ION_Crypto);

PHP_MINIT_FUNCTION(ION_DNS);
PHP_RINIT_FUNCTION(ION_DNS);
PHP_RSHUTDOWN_FUNCTION(ION_DNS);
PHP_MSHUTDOWN_FUNCTION(ION_DNS);

PHP_MINIT_FUNCTION(ION_FS);
PHP_RINIT_FUNCTION(ION_FS);
PHP_RSHUTDOWN_FUNCTION(ION_FS);
PHP_MSHUTDOWN_FUNCTION(ION_FS);

PHP_MINIT_FUNCTION(ION_Process);
PHP_RINIT_FUNCTION(ION_Process);
PHP_RSHUTDOWN_FUNCTION(ION_Process);
PHP_MSHUTDOWN_FUNCTION(ION_Process);

PHP_MINIT_FUNCTION(ION_Process_ExecResult);

PHP_MINIT_FUNCTION(ION_Process_IPC);

//PHP_MINIT_FUNCTION(ION_Process_Worker);
//PHP_RINIT_FUNCTION(ION_Process_Worker);
//PHP_RSHUTDOWN_FUNCTION(ION_Process_Worker);

PHP_MINIT_FUNCTION(ION_Stream);
PHP_RINIT_FUNCTION(ION_Stream);
PHP_RSHUTDOWN_FUNCTION(ION_Stream);
PHP_MSHUTDOWN_FUNCTION(ION_Stream);

PHP_MINIT_FUNCTION(ION);
PHP_RINIT_FUNCTION(ION);
PHP_RSHUTDOWN_FUNCTION(ION);
PHP_MSHUTDOWN_FUNCTION(ION);

PHP_MINIT_FUNCTION(ION_Listener);

PHP_MINIT_FUNCTION(ION_URI);

PHP_MINIT_FUNCTION(ION_Stream_StorageAbstract);
PHP_MINIT_FUNCTION(ION_Stream_Storage);
PHP_MINIT_FUNCTION(ION_Stream_Server);
PHP_MINIT_FUNCTION(ION_Stream_Client);

PHP_MINIT_FUNCTION(ION_HTTP_Message);
PHP_MINIT_FUNCTION(ION_HTTP_Request);
PHP_MINIT_FUNCTION(ION_HTTP_Response);
PHP_MINIT_FUNCTION(ION_HTTP_WebSocket_Frame);
PHP_MINIT_FUNCTION(ION_HTTP_WebSocketParser);
PHP_MINIT_FUNCTION(ION_HTTP);
PHP_MSHUTDOWN_FUNCTION(ION_HTTP);

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
