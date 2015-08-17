#include "Process.h"

DEFINE_CLASS(ION_Process);

/** public function ION\Process::fork(int $flags = 0, Stream[] &$ipc = null) : int */
CLASS_METHOD(ION_Process, fork) {

}

METHOD_ARGS_BEGIN(ION_Process, fork, 0)
    METHOD_ARG(flags, 0)
    METHOD_ARG(ipc, 1)
METHOD_ARGS_END()


/** public function ION\Process::signal($signo, $pid) : bool */
CLASS_METHOD(ION_Process, signal) {

}

METHOD_ARGS_BEGIN(ION_Process, signal, 0)
    METHOD_ARG(signo, 0)
    METHOD_ARG(pid, 0)
METHOD_ARGS_END()

/** public function ION\Process::getPid() : int */
CLASS_METHOD(ION_Process, getPid) {

}

METHOD_WITHOUT_ARGS(ION_Process, getPid);

/** public function ION\Process::getParentPid() : int */
CLASS_METHOD(ION_Process, getParentPid) {

}

METHOD_WITHOUT_ARGS(ION_Process, getParentPid);




CLASS_METHODS_START(ION_Process)
    METHOD(ION_Process, fork,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, signal, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getPid, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getParentPid, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Process) {
    PION_REGISTER_PLAIN_CLASS(ION_Process, "ION\\Process");
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Process) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Process) {
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Process) {
    return SUCCESS;
}