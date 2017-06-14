#include "ion.h"

zend_object_handlers ion_oh_ION_HTTP_WebSocket_Frame;
zend_class_entry * ion_ce_ION_HTTP_WebSocket_Frame;

zend_object * ion_http_websocket_frame_init(zend_class_entry * ce) {
    ion_http_websocket_frame * frame = ion_alloc_object(ce, ion_http_websocket_frame);

    return ion_init_object(ION_OBJECT_ZOBJ(frame), ce, &ion_oh_ION_HTTP_WebSocket_Frame);
}

void ion_http_websocket_frame_free(zend_object * object) {
    zend_object_std_dtor(object);

    ion_http_websocket_frame * frame = ION_ZOBJ_OBJECT(object, ion_http_websocket_frame);
    if(frame->body) {
        zend_string_release(frame->body);
    }
}

int ion_websocket_frame_header(websocket_parser * parser) {
    if(parser->data) {
        return 1;
    }
    zend_string * s = parser->data = zend_string_alloc(parser->length, 0);
    s->val[s->len] = '\000';
    return 0;
}

int ion_websocket_frame_body(websocket_parser * parser, const char *at, size_t size) {
    if(parser->flags & WS_HAS_MASK) {
        websocket_parser_decode(((zend_string*)parser->data)->val, at, size, parser);
    } else {
        memcpy(((zend_string*)parser->data)->val, at, size);
    }
    return 0;
}

/** public function ION\HTTP\WebSocket\Frame::parse(string $raw) */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, parse) {
    zend_string      * raw = NULL;
    zend_object      * frame_object = NULL;
    websocket_parser * parser;
    websocket_parser_settings   settings;
    ion_http_websocket_frame  * frame = NULL;


    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(raw)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    frame_object = pion_new_object_arg_0(zend_get_called_scope(execute_data));
    RETVAL_OBJ(frame_object);
    if(!frame_object) {
        return;
    }
    frame = ION_ZOBJ_OBJECT(frame_object, ion_http_websocket_frame);

    websocket_parser_settings_init(&settings);
    settings.on_frame_header = ion_websocket_frame_header;
    settings.on_frame_body = ion_websocket_frame_body;
    parser = emalloc(sizeof(websocket_parser));
    parser->data = NULL;
    websocket_parser_init(parser);
    websocket_parser_execute(parser, &settings, raw->val, raw->len);

    frame->flags = parser->flags;
    if(parser->data) {
        frame->body = parser->data;
    }
    if(frame->flags & WS_HAS_MASK) {
        memcpy(frame->mask, parser->mask, 4);
    }

    efree(parser);
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, parse, 1)
    METHOD_ARG_STRING(frame, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_HTTP_WebSocket_Frame, factory) {
    zend_long     opcode = WS_OP_TEXT;
    zend_string * body   = NULL;
    zend_long     flags  = WS_FIN;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_LONG(opcode)
        Z_PARAM_STR(body)
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, factory, 1)
    METHOD_ARG_STRING(body, 0)
    METHOD_ARG_LONG(flags, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::withOpcode(int $opcode) : static */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, withOpcode) {
    zend_long opcode = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(opcode)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ION_THIS_OBJECT(ion_http_websocket_frame)->flags = (websocket_flags)(opcode & WS_OP_MASK);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, withOpcode, 1)
    METHOD_ARG_LONG(opcode, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getOpcode() : int */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, getOpcode) {
    RETURN_LONG(ION_THIS_OBJECT(ion_http_websocket_frame)->flags & WS_OP_MASK);
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, getOpcode);

/** public function ION\HTTP\Message::withBody(string $body) : static */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, withBody) {
    zend_string * body = NULL;
    ion_http_websocket_frame * frame = ION_THIS_OBJECT(ion_http_websocket_frame);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(body)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(frame->body) {
        zend_string_release(frame->body);
    }
    frame->body = zend_string_copy(body);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, withBody, 1)
    METHOD_ARG_STRING(body, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getBody() : string */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, getBody) {
    ion_http_websocket_frame * frame = ION_THIS_OBJECT(ion_http_websocket_frame);

    if(frame->body) {
        RETURN_STR(zend_string_copy(frame->body));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, getBody);

/** public function ION\HTTP\Message::withFinalFlag(bool $flag) : static */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, withFinalFlag) {
    zend_bool flag = true;
    ion_http_websocket_frame * frame = ION_THIS_OBJECT(ion_http_websocket_frame);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_BOOL(flag)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(flag) {
        frame->flags |= WS_FIN;
    } else {
        frame->flags &= ~WS_FIN;
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, withFinalFlag, 1)
    METHOD_ARG_BOOL(flag, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getFinalFlag() : bool */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, getFinalFlag) {
    ion_http_websocket_frame * frame = ION_THIS_OBJECT(ion_http_websocket_frame);

    if(frame->flags & WS_FIN) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, getFinalFlag);

/** public function ION\HTTP\Message::hasMasking() : bool */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, hasMasking) {
    ion_http_websocket_frame * frame = ION_THIS_OBJECT(ion_http_websocket_frame);

    if(frame->flags & WS_HAS_MASK) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, hasMasking);

/** public function ION\HTTP\Message::getMasking() : int */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, getMasking) {
    ion_http_websocket_frame * frame = ION_THIS_OBJECT(ion_http_websocket_frame);

    if(frame->flags & WS_HAS_MASK) {
        RETURN_STRINGL(frame->mask, 4);
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, getMasking);

/** public function ION\HTTP\Message::withMasking(string $mask = null) : bool */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, withMasking) {
    zend_string * mask = NULL;
    ion_http_websocket_frame * frame = ION_THIS_OBJECT(ion_http_websocket_frame);

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(mask)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(mask && mask->len >= 4) {
        frame->flags |= WS_HAS_MASK;
        memcpy(frame->mask, mask->val, 4);
    } else {
        frame->mask[0] = (char) (((zend_ulong)frame >> 24) & 0xFF);
        frame->mask[1] = (char) (((zend_ulong)frame >> 16) & 0xFF);
        frame->mask[2] = (char) (((zend_ulong)frame >>  8) & 0xFF);
        frame->mask[3] = (char) (((zend_ulong)frame)       & 0xFF);
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, withMasking, 0)
    METHOD_ARG_STRING(mask, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::withoutMasking() : int */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, withoutMasking) {
    ion_http_websocket_frame * frame = ION_THIS_OBJECT(ion_http_websocket_frame);

    frame->flags &= ~WS_HAS_MASK;
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, withoutMasking);

/** public function ION\HTTP\WebSocket\Frame::parse(string $raw) */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, build) {
    ion_http_websocket_frame * frame = ION_THIS_OBJECT(ion_http_websocket_frame);
    zend_string * raw;
    size_t        len = websocket_calc_frame_size(frame->flags, frame->body->len);
    size_t        raw_len;

    raw = zend_string_alloc(len, 0);
    raw->val[len] = '\000';

    raw_len = websocket_build_frame(raw->val, frame->flags, frame->mask, frame->body->val, frame->body->len);
    ZEND_ASSERT(len == raw_len);
    RETURN_STR(raw);

}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, build)

CLASS_METHODS_START(ION_HTTP_WebSocket_Frame)
    METHOD(ION_HTTP_WebSocket_Frame, parse,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_WebSocket_Frame, factory,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_WebSocket_Frame, withOpcode,     ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, getOpcode,      ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, withBody,       ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, getBody,        ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, withFinalFlag,  ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, getFinalFlag,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, hasMasking,     ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, getMasking,     ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, withMasking,    ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, withoutMasking, ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, build,          ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_WebSocket_Frame) {
    pion_register_class(ION_HTTP_WebSocket_Frame, "ION\\HTTP\\WebSocket\\Frame", ion_http_websocket_frame_init, CLASS_METHODS(ION_HTTP_WebSocket_Frame));
    pion_init_std_object_handlers(ION_HTTP_WebSocket_Frame);
    pion_set_object_handler(ION_HTTP_WebSocket_Frame, free_obj, ion_http_websocket_frame_free);
    ion_class_set_offset(ion_oh_ION_HTTP_WebSocket_Frame, ion_http_websocket_frame);

    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_CONT",  WS_OP_CONTINUE);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_TEXT",  WS_OP_TEXT);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_BINARY",   WS_OP_BINARY);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_CLOSE", WS_OP_CLOSE);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_PING",  WS_OP_PING);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_PONG",  WS_OP_PONG);

    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OPCODE_MASK",  WS_OP_MASK);

    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "FLAG_NONE",    0x0);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "FLAG_MASKING", WS_FIN);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "FLAG_FINAL",   WS_HAS_MASK);

    return SUCCESS;
}