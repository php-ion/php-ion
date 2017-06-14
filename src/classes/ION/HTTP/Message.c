#include "ion.h"
#include <ext/standard/php_string.h>

zend_object_handlers ion_oh_ION_HTTP_Message;
zend_class_entry * ion_ce_ION_HTTP_Message;

zend_object * ion_http_message_init(zend_class_entry * ce) {
    ion_http_message * message = ion_alloc_object(ce, ion_http_message);
    ALLOC_HASHTABLE(message->headers);
    zend_hash_init(message->headers, 8, NULL, ZVAL_PTR_DTOR, 0);
    return ion_init_object(ION_OBJECT_ZOBJ(message), ce, &ion_oh_ION_HTTP_Message);
}

void ion_http_message_free(zend_object * zo_message) {
    zend_object_std_dtor(zo_message);

    ion_http_message * message = ION_ZOBJ_OBJECT(zo_message, ion_http_message);
    zend_hash_clean(message->headers);
    zend_hash_destroy(message->headers);
    FREE_HASHTABLE(message->headers);
    if(message->uri) {
        ion_object_release(message->uri);
    }
    if(message->reason) {
        zend_string_release(message->reason);
    }
    if(message->version) {
        zend_string_release(message->version);
    }
    if(message->body) {
        zend_string_release(message->body);
    }
    if(message->method) {
        zend_string_release(message->method);
    }
    if(message->target) {
        zend_string_release(message->target);
    }
}

/** public function ION\HTTP\Message::getProtocolVersion() : string */
CLASS_METHOD(ION_HTTP_Message, getProtocolVersion) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);
    if(message->version) {
        RETURN_STR(zend_string_copy(message->version));
    } else {
        RETURN_STRINGL(ION_HTTP_VERSION_DEFAULT, sizeof(ION_HTTP_VERSION_DEFAULT) - 1);
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_Message, getProtocolVersion);

/** public function ION\HTTP\Message::withProtocolVersion(string $version) : self */
CLASS_METHOD(ION_HTTP_Message, withProtocolVersion) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);
    zend_string      * version = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(version)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(message->version) {
        zend_string_release(message->version);
    }
    message->version = zend_string_copy(version);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, withProtocolVersion, 1)
    METHOD_ARG_STRING(version, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getHeaders() : array */
CLASS_METHOD(ION_HTTP_Message, getHeaders) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);

    RETURN_ARR(zend_array_dup(message->headers));
}

METHOD_WITHOUT_ARGS(ION_HTTP_Message, getHeaders);

/** public function ION\HTTP\Message::hasHeader(string $name) : bool */
CLASS_METHOD(ION_HTTP_Message, hasHeader) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);
    zend_string      * name = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    name = zend_string_tolower(name);
    if(zend_hash_exists(message->headers, name)) {
        RETVAL_TRUE;
    } else {
        RETVAL_FALSE;
    }
    zend_string_release(name);
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, hasHeader, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getHeader(string $name) : array */
CLASS_METHOD(ION_HTTP_Message, getHeader) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);
    zend_string      * name = NULL;
    zval             * header = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    name = zend_string_tolower(name);
    header = zend_hash_find(message->headers, name);
    zend_string_release(name);

    if(header) {
        RETURN_ZVAL(header, 1, 0)
    } else {
        array_init(return_value);
    }
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, getHeader, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getHeaderLine(string $name) : string */
CLASS_METHOD(ION_HTTP_Message, getHeaderLine) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);
    zend_string      * name = NULL;
    zval             * header = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    name = zend_string_tolower(name);
    header = zend_hash_find(message->headers, name);
    zend_string_release(name);

    if(header) {
        php_implode(ION_STR(ION_STR_COMA_SP), header, return_value);
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, getHeaderLine, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END();

void ion_http_message_store_header_value(zend_array * header, zval * value) {
    if(Z_TYPE_P(value) == IS_ARRAY) {
        zval * val;
        ZEND_HASH_FOREACH_VAL(Z_ARR_P(value), val)
            if(Z_TYPE_P(val) == IS_STRING) {
                zval_add_ref(val);
                zend_hash_next_index_insert(header, val);
            } else {
                zval v;
                ZVAL_COPY(&v, val);
                convert_to_string(&v);
                if(EG(exception)) {
                    zval_ptr_dtor(&v);
                    return;
                }
                zend_hash_next_index_insert(header, &v);
            }
        ZEND_HASH_FOREACH_END();
    } else if(Z_TYPE_P(value) == IS_STRING) {
        zval_add_ref(value);
        zend_hash_next_index_insert(header, value);
    } else {
        zval v;
        ZVAL_COPY(&v, value);
        convert_to_string(&v);
        if(EG(exception)) {
            zval_ptr_dtor(&v);
            return;
        }
        zend_hash_next_index_insert(header, &v);
    }
}

void ion_http_message_with_header(zend_array * headers, zend_string * name, zval * value) {
    zval header;

    name = zend_string_tolower(name);
    zend_hash_del(headers, name);
    array_init(&header);
    zend_hash_add(headers, name, &header);
    zend_string_release(name);
    ion_http_message_store_header_value(Z_ARR(header), value);
}

/** public function ION\HTTP\Message::withHeader(string $name, string|string[] $value) : self */
CLASS_METHOD(ION_HTTP_Message, withHeader) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);
    zend_string      * name = NULL;
    zval             * value = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(name)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ion_http_message_with_header(message->headers, name, value);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, withHeader, 1)
    METHOD_ARG_STRING(name, 0)
    METHOD_ARG(value, 0)
METHOD_ARGS_END();

void ion_http_message_with_added_header(zend_array * headers, zend_string * name, zval * value) {
    zval * header_ptr;

    name = zend_string_tolower(name);
    header_ptr = zend_hash_find(headers, name);
    if(!header_ptr) {
        zval header;
        array_init(&header);
        header_ptr = zend_hash_add(headers, name, &header);
    } else {
        ZVAL_COPY_VALUE(header_ptr, header_ptr);
    }
    zend_string_release(name);
    ion_http_message_store_header_value(Z_ARR_P(header_ptr), value);
}

/** public function ION\HTTP\Message::withAddedHeader(string $name, string|string[] $value) : self */
CLASS_METHOD(ION_HTTP_Message, withAddedHeader) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);
    zend_string      * name = NULL;
    zval             * value = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(name)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ion_http_message_with_added_header(message->headers, name, value);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, withAddedHeader, 1)
    METHOD_ARG_STRING(name, 0)
    METHOD_ARG(value, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::withoutHeader(string $name) : self */
CLASS_METHOD(ION_HTTP_Message, withoutHeader) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);
    zend_string      * name = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    name = zend_string_tolower(name);
    zend_hash_del(message->headers, name);
    zend_string_release(name);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, withoutHeader, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::withBody(string $body) : self */
CLASS_METHOD(ION_HTTP_Message, withBody) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);
    zend_string      * body = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(body)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(message->body) {
        zend_string_release(message->body);
    }
    message->body = zend_string_copy(body);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, withBody, 1)
    METHOD_ARG_STRING(body, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getBody() : string */
CLASS_METHOD(ION_HTTP_Message, getBody) {
    ion_http_message * message = ION_THIS_OBJECT(ion_http_message);

    if(message->body) {
        zend_string_addref(message->body);
        RETURN_STR(message->body);
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_Message, getBody);

CLASS_METHODS_START(ION_HTTP_Message)
    METHOD(ION_HTTP_Message, getProtocolVersion,  ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Message, withProtocolVersion, ZEND_ACC_PUBLIC)

    METHOD(ION_HTTP_Message, getHeaders,      ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Message, hasHeader,       ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Message, getHeader,       ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Message, getHeaderLine,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Message, withHeader,      ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Message, withAddedHeader, ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Message, withoutHeader,   ZEND_ACC_PUBLIC)

    METHOD(ION_HTTP_Message, withBody,        ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Message, getBody,         ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_Message) {
    pion_register_class(ION_HTTP_Message, "ION\\HTTP\\Message", ion_http_message_init, CLASS_METHODS(ION_HTTP_Message));
    pion_init_std_object_handlers(ION_HTTP_Message);
    pion_set_object_handler(ION_HTTP_Message, free_obj, ion_http_message_free);
    pion_set_object_handler(ION_HTTP_Message, clone_obj, NULL);
    ion_class_set_offset(ion_oh_ION_HTTP_Message, ion_http_message);

    return SUCCESS;
}