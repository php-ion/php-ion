#ifndef ION_STRINGS_H
#define ION_STRINGS_H

#define ION_HTTP_METHOD_STRINGS_OFFSET 0

#define ION_STR(x) GION(interned_strings)[x]

void ion_interned_strings_ctor(void);
void ion_interned_strings_dtor(void);

#define ION_INTERNED_STRINGS_MAP(XX) \
    XX(0, UP_DELETE      , "DELETE"         ) \
    XX(1, UP_GET         , "GET"            ) \
    XX(2, UP_HEAD        , "HEAD"           ) \
    XX(3, UP_POST        , "POST"           ) \
    XX(4, UP_PUT         , "PUT"            ) \
    XX(5, UP_CONNECT     , "CONNECT"        ) \
    XX(6, UP_OPTIONS     , "OPTIONS"        ) \
    XX(7, UP_TRACE       , "TRACE"          ) \
    XX(8, UP_COPY        , "COPY"           ) \
    XX(9, UP_LOCK        , "LOCK"           ) \
    XX(10, UP_MKCOL      , "MKCOL"          ) \
    XX(11, UP_MOVE       , "MOVE"           ) \
    XX(12, UP_PROPFIND   , "PROPFIND"       ) \
    XX(13, UP_PROPPATCH  , "PROPPATCH"      ) \
    XX(14, UP_SEARCH     , "SEARCH"         ) \
    XX(15, UP_UNLOCK     , "UNLOCK"         ) \
    XX(16, UP_BIND       , "BIND"           ) \
    XX(17, UP_REBIND     , "REBIND"         ) \
    XX(18, UP_UNBIND     , "UNBIND"         ) \
    XX(19, UP_ACL        , "ACL"            ) \
    XX(20, UP_REPORT     , "REPORT"         ) \
    XX(21, UP_MKACTIVITY , "MKACTIVITY"     ) \
    XX(22, UP_CHECKOUT   , "CHECKOUT"       ) \
    XX(23, UP_MERGE      , "MERGE"          ) \
    XX(24, UP_MSEARCH    , "M-SEARCH"       ) \
    XX(25, UP_NOTIFY     , "NOTIFY"         ) \
    XX(26, UP_SUBSCRIBE  , "SUBSCRIBE"      ) \
    XX(27, UP_UNSUBSCRIBE, "UNSUBSCRIBE"    ) \
    XX(28, UP_PATCH      , "PATCH"          ) \
    XX(29, UP_PURGE      , "PURGE"          ) \
    XX(30, UP_MKCALENDAR , "MKCALENDAR"     ) \
    XX(31, UP_LINK       , "LINK"           ) \
    XX(32, UP_UNLINK     , "UNLINK"         ) \
                                              \
    XX(35, COMA          , ","              ) \
    XX(36, COMA_SP       , ", "             ) \
    XX(37, SEMICOLON     , ";"              ) \
    XX(38, SEMICOLON_SP  , "; "             ) \
    XX(39, SLASH         , "/"              ) \
    XX(40, SLASH_SP      , "/ "             ) \
    XX(41, COLON         , ":"              ) \
    XX(42, COLON_SP      , ": "             ) \
    XX(43, SPACE         , " "              ) \
    XX(44, CRLF          , "\r\n"           ) \
                                              \
    XX(45, DONE          , "done"           ) \
    XX(46, CANCELED      , "canceled"       ) \
    XX(47, FAILED        , "failed"         ) \
    XX(48, IN_PROGRESS   , "in_progress"    ) \
    XX(49, PENDING       , "pending"        ) \
                                              \
    XX(50, V09           , "0.9"            ) \
    XX(51, V10           , "1.0"            ) \
    XX(52, V11           , "1.1"            ) \
    XX(53, V20           , "2.0"            ) \
                                              \
    XX(60, STREAM_STDIN  , "Stream(stdin)"  ) \
    XX(61, STREAM_STDOUT , "Stream(stdout)" ) \
    XX(62, STREAM_STDERR , "Stream(stderr)" ) \
                                              \
    XX(70, UP_HTTP       , "HTTP"           ) \
    XX(71, CONTENT_TYPE  , "content-type"   ) \

#endif //ION_STRINGS_H


enum ion_strings {
#define XX(num, name, string) ION_STR_##name = num,
    ION_INTERNED_STRINGS_MAP(XX)
#undef XX
};