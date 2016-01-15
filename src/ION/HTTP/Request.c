#include "../../pion.h"


zend_object_handlers ion_oh_ION_HTTP_Request;
zend_class_entry * ion_ce_ION_HTTP_Request;


static const char normalize_table[256] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel */
        0,       0,       0,       0,       0,       0,       0,       0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
        0,       1,       1,       1,       1,       1,       1,       1,
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
        1,       1,       1,       1,       1,       1,       1,       1,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
        0,       0,       1,       1,       1,       1,       1,       1,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
        1,       0,       0,       0,       0,       0,       0,       0,
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
        0,       0,       0,       1,       1,       1,       1,       1,
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
        1,      'A',     'B',     'C',     'D',     'E',     'F',     'G',
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
        'H',     'I',     'J',     'K',     'L',     'M',     'N',     'O',
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
        'P',     'Q',     'R',     'S',     'T',     'U',     'V',     'W',
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
        'X',     'Y',     'Z',      1,       1,      1,       1,       0 };

#define NT(t) normalize_table[(unsigned char) t]


CLASS_METHOD(ION_HTTP_Request, parse) {
    zend_string * request_string = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(request_string)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    RETURN_OBJ(pion_http_parse_request(request_string, zend_get_called_scope(execute_data)));
}

METHOD_ARGS_BEGIN(ION_HTTP_Request, parse, 1)
    METHOD_ARG_STRING(request, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Request::getURI() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, getURI) {
    ion_http_message * message = get_this_instance(ion_http_message);

    zend_object_addref(message->uri);
    RETURN_OBJ(message->uri);
}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, getURI)

/** public function ION\HTTP\Request::getMethod() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, getMethod) {
    ion_http_message * message = get_this_instance(ion_http_message);

    if(message->method) {
        zend_string_addref(message->method);
        RETURN_STR(message->method);
    } else {
        RETURN_STR(ION_STR(ION_STR_UP_GET));
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, getMethod)

/** public function ION\HTTP\Request::withMethod() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, withMethod) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * method = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(method)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(message->method) {
        zend_string_release(message->method);
    }

    message->method = zend_string_copy(method);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Request, withMethod, 1)
    METHOD_ARG_STRING(method, 0)
METHOD_ARGS_END();


/** public function ION\HTTP\Request::getRequestTarget() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, getRequestTarget) {
    ion_http_message * message = get_this_instance(ion_http_message);

    if(message->target) {
        zend_string_addref(message->target);
        RETURN_STR(message->target);
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, getRequestTarget)

/** public function ION\HTTP\Request::withRequestTarget() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, withRequestTarget) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * target = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(message->target) {
        zend_string_release(message->target);
    }

    message->target = zend_string_copy(target);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Request, withRequestTarget, 1)
    METHOD_ARG_STRING(target, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Request::__toString() : string */
CLASS_METHOD(ION_HTTP_Request, __toString) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string     ** pipe = emalloc(sizeof(zend_string *) * 32);
    size_t             pipe_size = 32;
    size_t             pipe_pos  = 0;
    size_t             message_size = 0;
    zend_string      * msg = NULL;
    zend_string      * str_space     = ION_STR(ION_STR_SPACE);
    zend_string      * str_http      = ION_STR(ION_STR_UP_HTTP);
    zend_string      * str_slash     = ION_STR(ION_STR_SLASH);
    zend_string      * str_crlf      = ION_STR(ION_STR_CRLF);
    zend_string      * str_colon_sp  = ION_STR(ION_STR_COLON_SP);
    zend_string      * str_up_get    = ION_STR(ION_STR_UP_GET);
    zend_string      * str_v11       = ION_STR(ION_STR_V11);
    zend_string      * str_coma      = ION_STR(ION_STR_COMA);
    zend_string      * target        = NULL;
    zend_string      * header = NULL;
    zval             * value = NULL;
    zval             * v = NULL;

    if(message->target) {
        target = zend_string_copy(message->target);
    } else if(message->uri) {
        target = ion_uri_stringify(message->uri, URI_ALL);
    }

    // begin head
    pipe[0] = message->method ? message->method : str_up_get;
    pipe[1] = str_space;
    message_size += pipe[0]->len + 1;
    pipe[2] = target;
    pipe[3] = str_space;
    pipe[4] = str_http;
    pipe[5] = str_slash;
    message_size += target->len + 6;
    pipe[6] = message->version ? message->version : str_v11;
    pipe[7] = str_crlf;
    message_size += pipe[6]->len + 2;
    pipe_pos = 8;

    ZEND_HASH_FOREACH_STR_KEY_VAL(message->headers, header, value) {
        if(zend_hash_num_elements(Z_ARR_P(value)) == 1) {
            if(pipe_pos + 4 > pipe_size) {
                pipe = erealloc(pipe, sizeof(zend_string *) * pipe_size * 2);
                pipe_size *= 2;
            }
            pipe[pipe_pos++] = header;
            pipe[pipe_pos++] = str_colon_sp;
            pipe[pipe_pos] = Z_STR_P(zend_hash_get_current_data(Z_ARR_P(value)));
            message_size += header->len + 4 + pipe[pipe_pos]->len;
            pipe_pos++;
            pipe[pipe_pos++] = str_crlf;
        } else {
            if(pipe_pos + 2 + zend_hash_num_elements(Z_ARR_P(value)) * 2 > pipe_size) {
                pipe = erealloc(pipe, sizeof(zend_string *) * pipe_size * 2);
                pipe_size *= 2;
            }
            pipe[pipe_pos++] = header;
            pipe[pipe_pos++] = str_colon_sp;
            message_size += header->len + 3;
            ZEND_HASH_FOREACH_VAL(Z_ARR_P(value), v) {
                pipe[pipe_pos++] = Z_STR_P(v);
                pipe[pipe_pos++] = str_coma;
                message_size += Z_STRLEN_P(v) + 1;
            } ZEND_HASH_FOREACH_END();
            pipe[pipe_pos-1] = str_crlf;
        }
    } ZEND_HASH_FOREACH_END();

    if(message->body) {
        message_size += message->body->len + 2;
    } else {
        message_size += 2;
    }

    msg = zend_string_alloc(message_size, 0);

    size_t offset = 0;
    zend_bool is_header = false;
    zend_bool to_up = false;
    for(size_t p = 0; p < pipe_pos; p++) {
        memcpy(&msg->val[offset], pipe[p]->val, pipe[p]->len);
        if(is_header) {
            for(size_t i = offset; i < offset + pipe[p]->len; i++) {
                if(NT(msg->val[i])) {
                    if(NT(msg->val[i]) == 1) {
                        to_up = true;
                    } else if(to_up) {
                        msg->val[i] = NT(msg->val[i]);
                        to_up = false;
                    }
                }
            }
            is_header = false;
        }
        offset += pipe[p]->len;
        if(pipe[p] == str_crlf) {
            is_header = true;
            to_up = true;
        }
    }
    memcpy(&msg->val[offset], "\r\n", 2);
    if(message->body) {
        memcpy(&msg->val[offset + 2], message->body->val, message->body->len);
    }

    efree(pipe);
    zend_string_release(target);
    msg->val[msg->len] = '\000';

    RETURN_STR(msg);
}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, __toString)

CLASS_METHODS_START(ION_HTTP_Request)
    METHOD(ION_HTTP_Request, parse,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_Request, getURI,      ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, getMethod,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, withMethod,  ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, withRequestTarget,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, getRequestTarget,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, __toString,  ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_Request) {
    pion_register_extended_class(ION_HTTP_Request, ion_ce_ION_HTTP_Message, "ION\\HTTP\\Request", ion_http_message_init, CLASS_METHODS(ION_HTTP_Request));

    pion_init_std_object_handlers(ION_HTTP_Request);
    pion_set_object_handler(ION_HTTP_Request, free_obj, ion_http_message_free);
    pion_set_object_handler(ION_HTTP_Request, clone_obj, NULL);

    return SUCCESS;
}