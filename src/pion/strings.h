#ifndef ION_STRINGS_H
#define ION_STRINGS_H

#define ION_HTTP_METHOD_STRINGS_OFFSET 0

#define ION_STR(x) GION(interned_strings)[x]

void ion_interned_strings_ctor(void);
void ion_interned_strings_dtor(void);

#define ION_INTERNED_STRINGS_MAP(XX) \
    XX(UP_DELETE     , "DELETE"         ) \
    XX(UP_GET        , "GET"            ) \
    XX(UP_HEAD       , "HEAD"           ) \
    XX(UP_POST       , "POST"           ) \
    XX(UP_PUT        , "PUT"            ) \
    XX(UP_CONNECT    , "CONNECT"        ) \
    XX(UP_OPTIONS    , "OPTIONS"        ) \
    XX(UP_TRACE      , "TRACE"          ) \
    XX(UP_COPY       , "COPY"           ) \
    XX(UP_LOCK       , "LOCK"           ) \
    XX(UP_MKCOL      , "MKCOL"          ) \
    XX(UP_MOVE       , "MOVE"           ) \
    XX(UP_PROPFIND   , "PROPFIND"       ) \
    XX(UP_PROPPATCH  , "PROPPATCH"      ) \
    XX(UP_SEARCH     , "SEARCH"         ) \
    XX(UP_UNLOCK     , "UNLOCK"         ) \
    XX(UP_BIND       , "BIND"           ) \
    XX(UP_REBIND     , "REBIND"         ) \
    XX(UP_UNBIND     , "UNBIND"         ) \
    XX(UP_ACL        , "ACL"            ) \
    XX(UP_REPORT     , "REPORT"         ) \
    XX(UP_MKACTIVITY , "MKACTIVITY"     ) \
    XX(UP_CHECKOUT   , "CHECKOUT"       ) \
    XX(UP_MERGE      , "MERGE"          ) \
    XX(UP_MSEARCH    , "M-SEARCH"       ) \
    XX(UP_NOTIFY     , "NOTIFY"         ) \
    XX(UP_SUBSCRIBE  , "SUBSCRIBE"      ) \
    XX(UP_UNSUBSCRIBE, "UNSUBSCRIBE"    ) \
    XX(UP_PATCH      , "PATCH"          ) \
    XX(UP_PURGE      , "PURGE"          ) \
    XX(UP_MKCALENDAR , "MKCALENDAR"     ) \
    XX(UP_LINK       , "LINK"           ) \
    XX(UP_UNLINK     , "UNLINK"         ) \
                                          \
    XX(COMA          , ","              ) \
    XX(COMA_SP       , ", "             ) \
    XX(SEMICOLON     , ";"              ) \
    XX(SEMICOLON_SP  , "; "             ) \
    XX(SLASH         , "/"              ) \
    XX(SLASH_SP      , "/ "             ) \
    XX(COLON         , ":"              ) \
    XX(COLON_SP      , ": "             ) \
    XX(SPACE         , " "              ) \
    XX(CRLF          , "\r\n"           ) \
                                          \
    XX(DONE          , "done"           ) \
    XX(CANCELED      , "canceled"       ) \
    XX(FAILED        , "failed"         ) \
    XX(IN_PROGRESS   , "in_progress"    ) \
    XX(PENDING       , "pending"        ) \
                                          \
    XX(V09           , "0.9"            ) \
    XX(V10           , "1.0"            ) \
    XX(V11           , "1.1"            ) \
    XX(V20           , "2.0"            ) \
                                          \
    XX(STREAM_STDIN  , "Stream(stdin)"  ) \
    XX(STREAM_STDOUT , "Stream(stdout)" ) \
    XX(STREAM_STDERR , "Stream(stderr)" ) \
                                          \
    XX(UP_HTTP       , "HTTP"           ) \
    XX(CONTENT_TYPE  , "content-type"   ) \
                                          \
    XX(HTTP_100,  "Continue")                        \
    XX(HTTP_101,  "Switching Protocols")             \
    XX(HTTP_102,  "Processing")                      \
    XX(HTTP_200,  "OK")                              \
    XX(HTTP_201,  "Created")                         \
    XX(HTTP_202,  "Accepted")                        \
    XX(HTTP_203,  "Non-Authoritative Information")   \
    XX(HTTP_204,  "No Content")                      \
    XX(HTTP_205,  "Reset Content")                   \
    XX(HTTP_206,  "Partial Content")                 \
    XX(HTTP_207,  "Multi-Status")                    \
    XX(HTTP_226,  "IM Used")                         \
    XX(HTTP_300,  "Multiple Choices")                \
    XX(HTTP_301,  "Moved Permanently")               \
    XX(HTTP_302,  "Moved Temporarily")               \
    XX(HTTP_303,  "See Other")                       \
    XX(HTTP_304,  "Not Modified")                    \
    XX(HTTP_305,  "Use Proxy")                       \
    XX(HTTP_307,  "Temporary Redirect")              \
    XX(HTTP_400,  "Bad Request")                     \
    XX(HTTP_401,  "Unauthorized")                    \
    XX(HTTP_402,  "Payment Required")                \
    XX(HTTP_403,  "Forbidden")                       \
    XX(HTTP_404,  "Not Found")                       \
    XX(HTTP_405,  "Method Not Allowed")              \
    XX(HTTP_406,  "Not Acceptable")                  \
    XX(HTTP_407,  "Proxy Authentication Required")   \
    XX(HTTP_408,  "Request Timeout")                 \
    XX(HTTP_409,  "Conflict")                        \
    XX(HTTP_410,  "Gone")                            \
    XX(HTTP_411,  "Length Required")                 \
    XX(HTTP_412,  "Precondition Failed")             \
    XX(HTTP_413,  "Request Entity Too Large")        \
    XX(HTTP_414,  "Request-URI Too Large")           \
    XX(HTTP_415,  "Unsupported Media Type")          \
    XX(HTTP_416,  "Requested Range Not Satisfiable") \
    XX(HTTP_417,  "Expectation Failed")              \
    XX(HTTP_418,  "I'm a teapot")                    \
    XX(HTTP_422,  "Unprocessable Entity")            \
    XX(HTTP_423,  "Locked")                          \
    XX(HTTP_424,  "Failed Dependency")               \
    XX(HTTP_425,  "Unordered Collection")            \
    XX(HTTP_426,  "Upgrade Required")                \
    XX(HTTP_428,  "Precondition Required")           \
    XX(HTTP_429,  "Too Many Requests")               \
    XX(HTTP_431,  "Request Header Fields Too Large") \
    XX(HTTP_434,  "Requested host unavailable")      \
    XX(HTTP_449,  "Retry With")                      \
    XX(HTTP_451,  "Unavailable For Legal Reasons")   \
    XX(HTTP_500,  "Internal Server Error")           \
    XX(HTTP_501,  "Not Implemented")                 \
    XX(HTTP_502,  "Bad Gateway")                     \
    XX(HTTP_503,  "Service Unavailable")             \
    XX(HTTP_504,  "Gateway Timeout")                 \
    XX(HTTP_505,  "HTTP Version Not Supported")      \
    XX(HTTP_506,  "Variant Also Negotiates")         \
    XX(HTTP_507,  "Insufficient Storage")            \
    XX(HTTP_508,  "Loop Detected")                   \
    XX(HTTP_509,  "Bandwidth Limit Exceeded")        \
    XX(HTTP_510,  "Not Extended")                    \
    XX(HTTP_511,  "Network Authentication Required") \
    XX(HTTP_NONE, "No Reason") \

#endif //ION_STRINGS_H


enum ion_strings {
#define XX(name, string) ION_STR_##name,
    ION_INTERNED_STRINGS_MAP(XX)
#undef XX
};