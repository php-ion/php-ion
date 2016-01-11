#include "../../pion.h"

zend_object_handlers ion_oh_ION_HTTP_Message;
zend_class_entry * ion_ce_ION_HTTP_Message;

zend_object * ion_http_message_init(zend_class_entry * ce) {
    ion_http_message * message = ecalloc(1, sizeof(ion_http_message));
    zend_hash_init(message->headers, 16, NULL, NULL, 0);
    message->version = zend_string_init(STRARGS("1.1"), 0);
    RETURN_INSTANCE(ION_HTTP_Message, message);
}

void ion_http_message_free(zend_object * zo_message) {
    zend_object_std_dtor(zo_message);

    ion_http_message * message = get_object_instance(zo_message, ion_http_message);
    zend_array_destroy(message->headers);
    if(message->uri) {
        zend_object_release(message->uri);
    }
    if(message->reason) {
        zend_string_release(message->reason);
    }
    if(message->version) {
        zend_string_release(message->version);
    }
}

/** public function ION\HTTP\Message::getProtocolVersion() : string */
CLASS_METHOD(ION_HTTP_Message, getProtocolVersion) {
    ion_http_message * message = get_this_instance(ion_http_message);
    RETURN_STR(zend_string_copy(message->version));
}

METHOD_WITHOUT_ARGS(ION_HTTP_Message, getProtocolVersion);

/** public function ION\HTTP\Message::withProtocolVersion(string $version) : self */
CLASS_METHOD(ION_HTTP_Message, withProtocolVersion) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * version = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(version)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    zend_string_release(message->version);
    message->version = zend_string_copy(version);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, withProtocolVersion, 1)
    METHOD_ARG_STRING(version, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getHeaders() : array */
CLASS_METHOD(ION_HTTP_Message, getHeaders) {
    ion_http_message * message = get_this_instance(ion_http_message);

    RETURN_ARR(zend_array_dup(message->headers));
}

METHOD_WITHOUT_ARGS(ION_HTTP_Message, getHeaders);

/** public function ION\HTTP\Message::hasHeader(string $name) : bool */
CLASS_METHOD(ION_HTTP_Message, hasHeader) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * name = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(zend_hash_exists(message->headers, name)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, hasHeader, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getHeader(string $name) : array */
CLASS_METHOD(ION_HTTP_Message, getHeader) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * name = NULL;
    zval             * header = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    header = zend_hash_find(message->headers, name);
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
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * name = NULL;
    zend_long          pos = 0;
    zval             * header = NULL;
    zval             * entry = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(name)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(pos)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    header = zend_hash_find(message->headers, name);
    if(header) {
        entry = zend_hash_index_find(Z_ARR_P(header), (zend_ulong)pos);
        RETURN_ZVAL(entry, 1, 0);
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, getHeaderLine, 1)
    METHOD_ARG_STRING(name, 0)
    METHOD_ARG_LONG(pos, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::withHeader(string $name, string|string[] $value) : self */
CLASS_METHOD(ION_HTTP_Message, withHeader) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * name = NULL;
    zval             * header = NULL;
    zval             * value = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(name)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    name = zend_string_tolower(name);
    zend_hash_del(message->headers, name);
    array_init(header);
    zend_hash_add(message->headers, name, header);
    if(Z_TYPE_P(value) == IS_ARRAY) {
        zval * new_header;
        ZEND_HASH_FOREACH_VAL(Z_ARR_P(value), new_header)
            convert_to_string(new_header);
            zval_add_ref(value);
            zend_hash_next_index_insert(Z_ARR_P(header), new_header);
        ZEND_HASH_FOREACH_END();
    } else {
        convert_to_string(value);
        zval_add_ref(value);
        zend_hash_next_index_insert(Z_ARR_P(header), value);
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, withHeader, 1)
    METHOD_ARG_STRING(name, 0)
    METHOD_ARG(value, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::withAddedHeader(string $name, string|string[] $value) : self */
CLASS_METHOD(ION_HTTP_Message, withAddedHeader) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * name = NULL;
    zval             * header = NULL;
    zval             * value = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_STR(name)
            Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    name = zend_string_tolower(name);
    header = zend_hash_find(message->headers, name);
    if(!header) {
        array_init(header);
        zend_hash_add(message->headers, name, header);
    }
    if(Z_TYPE_P(value) == IS_ARRAY) {
        zval * new_header;
        ZEND_HASH_FOREACH_VAL(Z_ARR_P(value), new_header)
                convert_to_string(new_header);
                zval_add_ref(value);
                zend_hash_next_index_insert(Z_ARR_P(header), new_header);
        ZEND_HASH_FOREACH_END();
    } else {
        convert_to_string(value);
        zval_add_ref(value);
        zend_hash_next_index_insert(Z_ARR_P(header), value);
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, withAddedHeader, 1)
    METHOD_ARG_STRING(name, 0)
    METHOD_ARG(value, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::withoutHeader(string $name) : self */
CLASS_METHOD(ION_HTTP_Message, withoutHeader) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * name = NULL;
    zval             * value = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_STR(name)
            Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    name = zend_string_tolower(name);
    zend_hash_del(message->headers, name);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Message, withoutHeader, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END();

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
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_Message) {
    pion_register_class(ION_HTTP_Message, "ION\\HTTP\\Message", ion_http_message_init, CLASS_METHODS(ION_HTTP_Message));
    pion_init_std_object_handlers(ION_HTTP_Message);
    pion_set_object_handler(ION_HTTP_Message, free_obj, ion_http_message_free);
    pion_set_object_handler(ION_HTTP_Message, clone_obj, NULL);

    return SUCCESS;
}