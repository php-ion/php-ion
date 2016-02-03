#ifndef ION_STRINGS_H
#define ION_STRINGS_H

#include <php.h>

#define ION_HTTP_METHOD_STRINGS_OFFSET 0

#define ION_HTTP_HEADERS_STRINGS_OFFSET ION_STR_HTTP_ACCEPT
#define ION_HTTP_HEADERS_STRINGS_LENGTH (ION_STR_HTTP_X_WEBKIT_CSP - ION_STR_HTTP_ACCEPT)

#define ION_HTTP_HEADERS_LOW_STRINGS_OFFSET ION_STR_HTTP_LOW_ACCEPT
#define ION_HTTP_HEADERS_LOW_STRINGS_LENGTH (ION_STR_HTTP_LOW_X_WEBKIT_CSP - ION_STR_HTTP_LOW_ACCEPT)

#define ION_HTTP_HEADERS_STRINGS_OFFSET_TO_LOW (ION_HTTP_HEADERS_LOW_STRINGS_OFFSET - ION_HTTP_HEADERS_STRINGS_OFFSET)

#define USE_ION_CACHE zend_ion_global_cache * ion_cache = GION(cache);

#define ION_STR(x) GION(cache)->interned_strings[x]
#define ION_STR_CACHE(x) ion_cache->interned_strings[x]

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
    XX(CRLFCRLF      , "\r\n\r\n"       ) \
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
    XX(HTTP_NONE, "No Reason")                       \
                                                     \
    XX(HTTP_ACCEPT                      , "Accept"                       ) \
    XX(HTTP_ACCEPT_CHARSET              , "Accept-Charset"               ) \
    XX(HTTP_ACCEPT_DATETIME             , "Accept-Datetime"              ) \
    XX(HTTP_ACCEPT_ENCODING             , "Accept-Encoding"              ) \
    XX(HTTP_ACCEPT_LANGUAGE             , "Accept-Language"              ) \
    XX(HTTP_ACCEPT_PATCH                , "Accept-Patch"                 ) \
    XX(HTTP_ACCEPT_RANGES               , "Accept-Ranges"                ) \
    XX(HTTP_ACCESS_CONTROL_ALLOW_ORIGIN , "Access-Control-Allow-Origin"  ) \
    XX(HTTP_AGE                         , "Age"                          ) \
    XX(HTTP_ALLOW                       , "Allow"                        ) \
    XX(HTTP_ALTERNATES                  , "Alternates"                   ) \
    XX(HTTP_AUTHORIZATION               , "Authorization"                ) \
    XX(HTTP_CACHE_CONTROL               , "Cache-Control"                ) \
    XX(HTTP_CONNECTION                  , "Connection"                   ) \
    XX(HTTP_CONTENT_BASE                , "Content-Base"                 ) \
    XX(HTTP_CONTENT_DISPOSITION         , "Content-Disposition"          ) \
    XX(HTTP_CONTENT_ENCODING            , "Content-Encoding"             ) \
    XX(HTTP_CONTENT_LANGUAGE            , "Content-Language"             ) \
    XX(HTTP_CONTENT_LENGTH              , "Content-Length"               ) \
    XX(HTTP_CONTENT_LOCATION            , "Content-Location"             ) \
    XX(HTTP_CONTENT_MD5                 , "Content-MD5"                  ) \
    XX(HTTP_CONTENT_RANGE               , "Content-Range"                ) \
    XX(HTTP_CONTENT_SECURITY_POLICY     , "Content-Security-Policy"      ) \
    XX(HTTP_CONTENT_TYPE                , "Content-Type"                 ) \
    XX(HTTP_CONTENT_VERSION             , "Content-Version"              ) \
    XX(HTTP_COOKIE                      , "Cookie"                       ) \
    XX(HTTP_DATE                        , "Date"                         ) \
    XX(HTTP_DERIVED_FROM                , "Derived-From"                 ) \
    XX(HTTP_ETAG                        , "ETag"                         ) \
    XX(HTTP_EXPECT                      , "Expect"                       ) \
    XX(HTTP_EXPIRES                     , "Expires"                      ) \
    XX(HTTP_FORWARDED                   , "Forwarded"                    ) \
    XX(HTTP_FROM                        , "From"                         ) \
    XX(HTTP_FRONT_END_HTTPS             , "Front-End-Https"              ) \
    XX(HTTP_HOST                        , "Host"                         ) \
    XX(HTTP_IF_MATCH                    , "If-Match"                     ) \
    XX(HTTP_IF_MODIFIED_SINCE           , "If-Modified-Since"            ) \
    XX(HTTP_IF_NONE_MATCH               , "If-None-Match"                ) \
    XX(HTTP_IF_RANGE                    , "If-Range"                     ) \
    XX(HTTP_IF_UNMODIFIED_SINCE         , "If-Unmodified-Since"          ) \
    XX(HTTP_LAST_MODIFIED               , "Last-Modified"                ) \
    XX(HTTP_LINK                        , "Link"                         ) \
    XX(HTTP_LOCATION                    , "Location"                     ) \
    XX(HTTP_MIME_VERSION                , "MIME-Version"                 ) \
    XX(HTTP_MAX_FORWARDS                , "Max-Forwards"                 ) \
    XX(HTTP_PERMANENT                   , "Permanent"                    ) \
    XX(HTTP_PRAGMA                      , "Pragma"                       ) \
    XX(HTTP_PROXY_AUTHENTICATE          , "Proxy-Authenticate"           ) \
    XX(HTTP_PROXY_AUTHORIZATION         , "Proxy-Authorization"          ) \
    XX(HTTP_PUBLIC                      , "Public"                       ) \
    XX(HTTP_RANGE                       , "Range"                        ) \
    XX(HTTP_REFERER                     , "Referer"                      ) \
    XX(HTTP_RETRY_AFTER                 , "Retry-After"                  ) \
    XX(HTTP_SERVER                      , "Server"                       ) \
    XX(HTTP_SET_COOKIE                  , "Set-Cookie"                   ) \
    XX(HTTP_STATUS                      , "Status"                       ) \
    XX(HTTP_STRICT_TRANSPORT_SECURITY   , "Strict-Transport-Security"    ) \
    XX(HTTP_TE                          , "TE"                           ) \
    XX(HTTP_TSV                         , "TSV"                          ) \
    XX(HTTP_TITLE                       , "Title"                        ) \
    XX(HTTP_TRAILER                     , "Trailer"                      ) \
    XX(HTTP_TRANSFER_ENCODING           , "Transfer-Encoding"            ) \
    XX(HTTP_URI                         , "URI"                          ) \
    XX(HTTP_UPGRADE                     , "Upgrade"                      ) \
    XX(HTTP_USER_AGENT                  , "User-Agent"                   ) \
    XX(HTTP_VARY                        , "Vary"                         ) \
    XX(HTTP_VIA                         , "Via"                          ) \
    XX(HTTP_WWW_AUTHENTICATE            , "WWW-Authenticate"             ) \
    XX(HTTP_WARNING                     , "Warning"                      ) \
    XX(HTTP_X_ATT_DEVICEID              , "X-ATT-DeviceId"               ) \
    XX(HTTP_X_CONTENT_SECURITY_POLICY   , "X-Content-Security-Policy"    ) \
    XX(HTTP_X_FORWARDED_FOR             , "X-Forwarded-For"              ) \
    XX(HTTP_X_FORWARDED_HOST            , "X-Forwarded-Host"             ) \
    XX(HTTP_X_POWERED_BY                , "X-Powered-By"                 ) \
    XX(HTTP_X_WAP_PROFILE               , "X-Wap-Profile"                ) \
    XX(HTTP_X_WEBKIT_CSP                , "X-WebKit-CSP"                 ) \
                                                                           \
    XX(HTTP_LOW_ACCEPT                          , "accept"                        )  \
    XX(HTTP_LOW_ACCEPT_CHARSET                  , "accept-charset"                )  \
    XX(HTTP_LOW_ACCEPT_DATETIME                 , "accept-datetime"               )  \
    XX(HTTP_LOW_ACCEPT_ENCODING                 , "accept-encoding"               )  \
    XX(HTTP_LOW_ACCEPT_LANGUAGE                 , "accept-language"               )  \
    XX(HTTP_LOW_ACCEPT_PATCH                    , "accept-patch"                  )  \
    XX(HTTP_LOW_ACCEPT_RANGES                   , "accept-ranges"                 )  \
    XX(HTTP_LOW_ACCESS_CONTROL_ALLOW_ORIGIN     , "access-control-allow-origin"   )  \
    XX(HTTP_LOW_AGE                             , "age"                           )  \
    XX(HTTP_LOW_ALLOW                           , "allow"                         )  \
    XX(HTTP_LOW_ALTERNATES                      , "alternates"                    )  \
    XX(HTTP_LOW_AUTHORIZATION                   , "authorization"                 )  \
    XX(HTTP_LOW_CACHE_CONTROL                   , "cache-control"                 )  \
    XX(HTTP_LOW_CONNECTION                      , "connection"                    )  \
    XX(HTTP_LOW_CONTENT_BASE                    , "content-base"                  )  \
    XX(HTTP_LOW_CONTENT_DISPOSITION             , "content-disposition"           )  \
    XX(HTTP_LOW_CONTENT_ENCODING                , "content-encoding"              )  \
    XX(HTTP_LOW_CONTENT_LANGUAGE                , "content-language"              )  \
    XX(HTTP_LOW_CONTENT_LENGTH                  , "content-length"                )  \
    XX(HTTP_LOW_CONTENT_LOCATION                , "content-location"              )  \
    XX(HTTP_LOW_CONTENT_MD5                     , "content-md5"                   )  \
    XX(HTTP_LOW_CONTENT_RANGE                   , "content-range"                 )  \
    XX(HTTP_LOW_CONTENT_SECURITY_POLICY         , "content-security-policy"       )  \
    XX(HTTP_LOW_CONTENT_TYPE                    , "content-type"                  )  \
    XX(HTTP_LOW_CONTENT_VERSION                 , "content-version"               )  \
    XX(HTTP_LOW_COOKIE                          , "cookie"                        )  \
    XX(HTTP_LOW_DATE                            , "date"                          )  \
    XX(HTTP_LOW_DERIVED_FROM                    , "derived-from"                  )  \
    XX(HTTP_LOW_ETAG                            , "etag"                          )  \
    XX(HTTP_LOW_EXPECT                          , "expect"                        )  \
    XX(HTTP_LOW_EXPIRES                         , "expires"                       )  \
    XX(HTTP_LOW_FORWARDED                       , "forwarded"                     )  \
    XX(HTTP_LOW_FROM                            , "from"                          )  \
    XX(HTTP_LOW_FRONT_END_HTTPS                 , "front-end-https"               )  \
    XX(HTTP_LOW_HOST                            , "host"                          )  \
    XX(HTTP_LOW_IF_MATCH                        , "if-match"                      )  \
    XX(HTTP_LOW_IF_MODIFIED_SINCE               , "if-modified-since"             )  \
    XX(HTTP_LOW_IF_NONE_MATCH                   , "if-none-match"                 )  \
    XX(HTTP_LOW_IF_RANGE                        , "if-range"                      )  \
    XX(HTTP_LOW_IF_UNMODIFIED_SINCE             , "if-unmodified-since"           )  \
    XX(HTTP_LOW_LAST_MODIFIED                   , "last-modified"                 )  \
    XX(HTTP_LOW_LINK                            , "link"                          )  \
    XX(HTTP_LOW_LOCATION                        , "location"                      )  \
    XX(HTTP_LOW_MIME_VERSION                    , "mime-version"                  )  \
    XX(HTTP_LOW_MAX_FORWARDS                    , "max-forwards"                  )  \
    XX(HTTP_LOW_PERMANENT                       , "permanent"                     )  \
    XX(HTTP_LOW_PRAGMA                          , "pragma"                        )  \
    XX(HTTP_LOW_PROXY_AUTHENTICATE              , "proxy-authenticate"            )  \
    XX(HTTP_LOW_PROXY_AUTHORIZATION             , "proxy-authorization"           )  \
    XX(HTTP_LOW_PUBLIC                          , "public"                        )  \
    XX(HTTP_LOW_RANGE                           , "range"                         )  \
    XX(HTTP_LOW_REFERER                         , "referer"                       )  \
    XX(HTTP_LOW_RETRY_AFTER                     , "retry-after"                   )  \
    XX(HTTP_LOW_SERVER                          , "server"                        )  \
    XX(HTTP_LOW_SET_COOKIE                      , "set-cookie"                    )  \
    XX(HTTP_LOW_STATUS                          , "status"                        )  \
    XX(HTTP_LOW_STRICT_TRANSPORT_SECURITY       , "strict-transport-security"     )  \
    XX(HTTP_LOW_TE                              , "te"                            )  \
    XX(HTTP_LOW_TSV                             , "tsv"                           )  \
    XX(HTTP_LOW_TITLE                           , "title"                         )  \
    XX(HTTP_LOW_TRAILER                         , "trailer"                       )  \
    XX(HTTP_LOW_TRANSFER_ENCODING               , "transfer-encoding"             )  \
    XX(HTTP_LOW_URI                             , "uri"                           )  \
    XX(HTTP_LOW_UPGRADE                         , "upgrade"                       )  \
    XX(HTTP_LOW_USER_AGENT                      , "user-agent"                    )  \
    XX(HTTP_LOW_VARY                            , "vary"                          )  \
    XX(HTTP_LOW_VIA                             , "via"                           )  \
    XX(HTTP_LOW_WWW_AUTHENTICATE                , "www-authenticate"              )  \
    XX(HTTP_LOW_WARNING                         , "warning"                       )  \
    XX(HTTP_LOW_X_ATT_DEVICEID                  , "x-att-deviceid"                )  \
    XX(HTTP_LOW_X_CONTENT_SECURITY_POLICY       , "x-content-security-policy"     )  \
    XX(HTTP_LOW_X_FORWARDED_FOR                 , "x-forwarded-for"               )  \
    XX(HTTP_LOW_X_FORWARDED_HOST                , "x-forwarded-host"              )  \
    XX(HTTP_LOW_X_POWERED_BY                    , "x-powered-by"                  )  \
    XX(HTTP_LOW_X_WAP_PROFILE                   , "x-wap-profile"                 )  \
    XX(HTTP_LOW_X_WEBKIT_CSP                    , "x-webkit-csp"                  )  \
                   \
    XX(LAST , "" ) \

#endif //ION_STRINGS_H


enum ion_strings {
#define XX(name, string) ION_STR_##name,
    ION_INTERNED_STRINGS_MAP(XX)
#undef XX
};