#ifndef PION_ERRORS_H
#define PION_ERRORS_H

// class ION
#define ERR_ION_DISPATCH_IN_LOOP    "Dispatching in progress"
#define ERR_ION_DISPATCH_FAILED     "Dispatching runtime error"
#define ERR_ION_AWAIT_INVALID_TIME  "Timeout should be unsigned"
#define ERR_ION_AWAIT_FAILED        "Unable to add event to queue"

// class URI
#define ERR_ION_HTTP_URI_FACTORY_UNKNOWN "Unknown option %d"

// class ION\HTTP\Request
#define ERR_ION_HTTP_REQUEST_FACTORY_URI    "URI should be instance of ION\\URI"
#define ERR_ION_HTTP_REQUEST_FACTORY_METHOD "Method should be a string"
#define ERR_ION_HTTP_REQUEST_FACTORY_HEADERS "Headers should be an array"
#define ERR_ION_HTTP_REQUEST_FACTORY_UNKNOWN "Unknown option %d"

// class ION\HTTP\Response
#define ERR_ION_HTTP_RESPONSE_UNKNOWN_STATUS "Unknown status %d"
#define ERR_ION_HTTP_RESPONSE_FACTORY_STATUS "Method should be an integer"
#define ERR_ION_HTTP_RESPONSE_FACTORY_UNKNOWN "Unknown option %d"

// class ION\DNS
#define ERR_ION_DNS_RESOLV_NOT_FOUND "ion.dns.resolv_conf: failed to open file %s"
#define ERR_ION_DNS_RESOLV_CANT_STAT "ion.dns.resolv_conf: failed to stat file %s"
#define ERR_ION_DNS_RESOLV_TOO_LARGE "ion.dns.resolv_conf: file %s too large"
#define ERR_ION_DNS_RESOLV_OUT_OF_MEMORY "ion.dns.resolv_conf: out of memory"
#define ERR_ION_DNS_RESOLV_CANT_READ "ion.dns.resolv_conf: short read from file %s"
#define ERR_ION_DNS_RESOLV_NO_SERVERS "ion.dns.resolv_conf: no nameservers listed in the file %s"
#define ERR_ION_DNS_RESOLV_ERROR "ion.dns.resolv_conf: unknown error occurred while reading file %s"
#define ERR_ION_DNS_RESOLVE_NOT_STORED "Failed to store DNS request"
#define ERR_ION_DNS_RESOLVE_NOT_CLEANED "Failed to remove a addrinfo request"
#define ERR_ION_DNS_RESOLVE_FAILED "DNS request failed: %s"

// class ION\FS
#define ERR_ION_FS_READ_BROKEN_PIPE "Failed to read file: buffer is unreachble"
#define ERR_ION_FS_READ_CANT_OPEN "Failed to open file %s: %s"
#define ERR_ION_FS_READ_CANT_STAT "Stat failed for %s: %s"
#define ERR_ION_FS_READ_CANT_OPEN_PIPE "Failed to create pair stream"
#define ERR_ION_FS_READ_SENDFILE_FAILED "Failed to transfer data via sendfile/mmap"
#define ERR_ION_FS_READ_FILE_NOT_FOUND "File or directory %s doesn't exists"
#define ERR_ION_FS_READ_CANT_STORE_WATCHER "Failed to store watcher"
#define ERR_ION_FS_READ_CANT_LISTEN_EVENTS "Failed to open file %s for events: %s"

// class ION\Process

#endif //PION_ERRORS_H
