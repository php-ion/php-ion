#ifndef ION_ERRORS_H
#define ION_ERRORS_H


// class ION
#define ERR_ION_DISPATCH_IN_LOOP    "Dispatching in progress"
#define ERR_ION_DISPATCH_FAILED     "Dispatching runtime error"
#define ERR_ION_AWAIT_INVALID_TIME  "Timeout should be unsigned"
#define ERR_ION_AWAIT_FAILED        "Unable to add event to queue"
#define ERR_ION_REINIT_FAILED       "Some events could not be restarted"

// class ION/EventAbstract
#define ERR_ION_EVENT_NOT_READY     "This event doesn't initialized"
#define ERR_ION_EVENT_ADD           "Unable to activate event"

// class ION\Promise, ION\Deferred, ION\ResolvablePromise, ION\Sequence
#define ERR_ION_PROMISE_ITSELF           "Can not promise itself"
#define ERR_ION_PROMISE_CANT             "Can't promise"
#define ERR_ION_PROMISE_ONLY_PROMISES    "Handler should be a valid promise-like object or string"
#define ERR_ION_PROMISE_CLONE_INTERNAL   "Trying to clone an internal promisor"
#define ERR_ION_PROMISE_YIELDED          "Promisor in progress"
#define ERR_ION_PROMISE_ALREADY_FINISHED "Deferred has been finished"
#define ERR_ION_PROMISE_FINISH_INTERNAL  "Internal promisor could not be finished from userspace"
#define ERR_ION_PROMISE_INVALID_CONSEQ   "Promise expects a valid callbacks"

// class ION\URI
#define ERR_ION_URI_PARSE_FAILED     "URI could not be parsed"
#define ERR_ION_URI_USER_INFO_FAILED "Failed to generate user info"

// class ION\Crypto
#define ERR_ION_CRYPTO_SSL2_DISABLED          "SSLv2 unavailable in the OpenSSL library against which PHP is linked"
#define ERR_ION_CRYPTO_INVALID_METHOD         "Invalid crypt method"
#define ERR_ION_CRYPTO_NO_METHOD              "No method is specified"
#define ERR_ION_CRYPTO_INIT_FAILED            "SSL/TLS context creation failure"
#define ERR_ION_CRYPTO_DEF_VERIFY_FAILED      "Unable to set default verify locations"
#define ERR_ION_CRYPTO_CERT_CHAIN_FAILED      "Unable to set local cert chain file '%s'; Check that your cafile/capath settings include details of your certificate and its issuer"
#define ERR_ION_CRYPTO_PKEY_FAILED            "Unable to set private key file '%s'%s"
#define ERR_ION_CRYPTO_PKEY_FAILED_PASSPHRASE " (passphrase required; set passphrase before)"
#define ERR_ION_CRYPTO_PKEY_MISSED            "Private key does not match certificate!"
#define ERR_ION_CRYPTO_CAFILE_FAILED          "Failed loading CA names from cafile %s"
#define ERR_ION_CRYPTO_LOAD_VERIFY_FAILED     "Unable to load verify locations %s, %s"
#define ERR_ION_CRYPTO_ENTROPY_FAILED         "Crypto: failed to generate entropy"

// class ION\Stream
#define ERR_ION_STREAM_RESOURCE_INVALID        "Argument must be either valid PHP stream resource"
#define ERR_ION_STREAM_RESOURCE_DUP_FAILED     "Failed to duplicate fd: %s"
#define ERR_ION_STREAM_CREATE_BUFFER_CORRUPTED "Failed to create Stream: buffer corrupted"
#define ERR_ION_STREAM_CRYPTO_FAILED           "Failed to setup SSL/TLS handler for stream %s"
#define ERR_ION_STREAM_HOST_INVALID            "Host or socket %s is not well-formed"
#define ERR_ION_STREAM_WRITE_FAILED            "Failed to write data"
#define ERR_ION_STREAM_READ_FAILED             "Stream buffer is unreachable"
#define ERR_ION_STREAM_SET_ENCRYPT_FAILED      "Failed to setup SSL/TLS handler"
#define ERR_ION_STREAM_ALREADY_ENCRYPTED       "Stream already encrypted"
#define ERR_ION_STREAM_CONNECT_FAILED          "Failed to connect to %s: %s"
#define ERR_ION_STREAM_PATH_TOO_LONG           "Path %s too long"
#define ERR_ION_STREAM_PATH_INVALID            "Failed to open socket %s: No such directory"
#define ERR_ION_STREAM_PRIORITY_INVALID        "Invalid priority value"
#define ERR_ION_STREAM_NEGATIVE                "The number of bytes cannot be negative"
#define ERR_ION_STREAM_TOKEN_EMPTY             "Empty token string"
#define ERR_ION_STREAM_SEARCH_FAILED           "Failed to get internal buffer pointer for token_length/offset"
#define ERR_ION_STREAM_BUFFER_ID_INVALID       "Invalid buffer identify"
#define ERR_ION_STREAM_BUFFER_CORRUPTED        "Stream buffer is corrupted"
#define ERR_ION_STREAM_READ_LOCKED             "Stream locked for reading: already in the process of reading"
#define ERR_ION_STREAM_APPEND_FAILED           "Failed to append data to input"
#define ERR_ION_STREAM_DRAIN_FAILED            "Failed to drain token"

// class ION\Listener
#define ERR_ION_LISTENER_UNSUPPORTED_ADDRESS  "Address family %d not supported by protocol family"
#define ERR_ION_LISTENER_INVALID_FORMAT       "Address %s is not well-formed"
#define ERR_ION_LISTENER_FAILED_OPEN_SOCKET   "Failed to open socket %s: %s"
#define ERR_ION_LISTENER_FAILED_LISTEN_SOCKET "Failed to listen on %s: %s"
#define ERR_ION_LISTENER_GOT_AN_ERROR         "Got an error %d (%s) on the listener %s. Shutting down listener."
#define ERR_ION_LISTENER_SSL_ERROR            "Failed to create SSL/TLS handler for incoming connection to %s. Connection will be refused."

// class ION\URI
#define ERR_ION_HTTP_URI_FACTORY_UNKNOWN      "Unknown option %d"

// class ION\HTTP
#define ERR_ION_HTTP_REQUEST_INVALID_STREAM   "Invalid stream (closed or corrupted)"

// class ION\HTTP\Request
#define ERR_ION_HTTP_REQUEST_FACTORY_URI     "URI should be instance of ION\\URI"
#define ERR_ION_HTTP_REQUEST_FACTORY_METHOD  "Method should be a string"
#define ERR_ION_HTTP_REQUEST_FACTORY_HEADERS "Headers should be an array"
#define ERR_ION_HTTP_REQUEST_FACTORY_UNKNOWN "Unknown option %d"

// class ION\HTTP\Response
#define ERR_ION_HTTP_RESPONSE_UNKNOWN_STATUS  "Unknown status %d"
#define ERR_ION_HTTP_RESPONSE_FACTORY_STATUS  "Method should be an integer"
#define ERR_ION_HTTP_RESPONSE_FACTORY_UNKNOWN "Unknown option %d"

// class ION\DNS
#define ERR_ION_DNS_RESOLV_NOT_FOUND        "ion.dns.resolv_conf: failed to open file %s (use synchronous dns)"
#define ERR_ION_DNS_RESOLV_CANT_STAT        "ion.dns.resolv_conf: failed to stat file %s (use synchronous dns)"
#define ERR_ION_DNS_RESOLV_TOO_LARGE        "ion.dns.resolv_conf: file %s too large (use synchronous dns)"
#define ERR_ION_DNS_RESOLV_OUT_OF_MEMORY    "ion.dns.resolv_conf: out of memory (use synchronous dns)"
#define ERR_ION_DNS_RESOLV_CANT_READ        "ion.dns.resolv_conf: short read from file %s (use synchronous dns)"
#define ERR_ION_DNS_RESOLV_NO_SERVERS       "ion.dns.resolv_conf: no nameservers listed in the file %s (use synchronous dns)"
#define ERR_ION_DNS_RESOLV_ERROR            "ion.dns.resolv_conf: unknown error occurred while reading file %s (use synchronous dns)"
#define ERR_ION_DNS_RESOLVE_NOT_STORED      "Failed to store DNS request"
#define ERR_ION_DNS_RESOLVE_NOT_CLEANED     "Failed to remove a addrinfo request"
#define ERR_ION_DNS_RESOLVE_FAILED          "DNS request failed: %s"

// class ION\FS
#define ERR_ION_FS_READ_BROKEN_PIPE         "Failed to read file: buffer is unreachble"
#define ERR_ION_FS_READ_CANT_OPEN           "Failed to open file %s: %s"
#define ERR_ION_FS_READ_CANT_STAT           "Stat failed for %s: %s"
#define ERR_ION_FS_READ_CANT_OPEN_PIPE      "Failed to create pair stream"
#define ERR_ION_FS_READ_SENDFILE_FAILED     "Failed to transfer data via sendfile/mmap"
#define ERR_ION_FS_EVENT_FILE_NOT_FOUND     "File or directory %s doesn't exists"
#define ERR_ION_FS_EVENT_CANT_STORE_WATCHER "Failed to store watcher"
#define ERR_ION_FS_EVENT_CANT_LISTEN_EVENTS "Failed to open file %s for events: %s"
#define ERR_ION_FS_EVENT_NO_QUEUE           "FS watcher: Could not open watchers queue: %s"
#define ERR_ION_FS_EVENT_CANT_ADD_EVENT     "Failed to add fsnotify event"
#define ERR_ION_FS_EVENT_CANT_DELETE_EVENT  "FS watcher: Could not remove watcher from queue: %s"
#define ERR_ION_FS_EVENT_ERROR              "FS watcher: An error occurred: %s"
#define ERR_ION_FS_INOTIFY_INIT_FAILED      "Failed to init inotify for file %s: %s"
#define ERR_ION_FS_INOTIFY_ADD_FAILED       "Failed to listen of %s"

// class ION\Process
#define ERR_ION_PROCESS_SIGNAL_EVENT_FAIL  "Failed to listening signal %d"
#define ERR_ION_PROCESS_SIGNAL_STORE_FAIL  "Failed to register signal (%d) handler"
#define ERR_ION_PROCESS_INVALID_UID        "Invalid user identifier"
#define ERR_ION_PROCESS_INVALID_GID        "Invalid group identifier"
#define ERR_ION_PROCESS_NO_USER_INFO_NAMED "Failed to get info by user name %s: %s"
#define ERR_ION_PROCESS_NO_USER_INFO_UID   "Failed to get info by UID %d: %s"
#define ERR_ION_PROCESS_GET_GID            "Failed to set GID %d: %s"
#define ERR_ION_PROCESS_GET_UID            "Failed to set UID %d: %s"
#define ERR_ION_PROCESS_GET_PRIO           "Failed to get process %d priority: %s"
#define ERR_ION_PROCESS_INVALID_PRIO       "Invalid priority value"
#define ERR_ION_PROCESS_PRIO_FAILED        "Failed to set process %d priority %d: %s"
#define ERR_ION_PROCESS_EXEC_NO_STDOUT     "Failed to initializate stdout stream: %s"
#define ERR_ION_PROCESS_EXEC_NO_STDERR     "Failed to initializate stderr stream: %s"
#define ERR_ION_PROCESS_EXEC_FORK_FAIL     "Failed to spawn process for command: %s"
#define ERR_ION_PROCESS_EXEC_SET_GID_FAIL  "Failed to set GID %d: %s\n"
#define ERR_ION_PROCESS_EXEC_SET_UID_FAIL  "Failed to set UID %d: %s\n"
#define ERR_ION_PROCESS_EXEC_CHDIR_FAIL    "Failed to change cwd to %s: %s\n"
#define ERR_ION_PROCESS_SPAWN_FAIL         "Failed to spawn process: %s"

// class ION\Process\ChildProcess
#define ERR_ION_PROCESS_IPC_FAIL           "Failed to create IPC channel"

#endif //ION_ERRORS_H
