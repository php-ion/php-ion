#include "../../../pion.h"

zend_object_handlers ion_oh_ION_HTTP_WebSocket_Frame;
zend_class_entry * ion_ce_ION_HTTP_WebSocket_Frame;

#define WS_OPCODE_MASK  0x0F
#define WS_FLAG_FIN     0x10
#define WS_FLAG_MASKED  0x20


int ion_websocket_frame_body(websocket_parser * parser, const char *at, size_t size) {

    return 0;
}

CLASS_METHOD(ION_HTTP_WebSocket_Frame, parse) {
    zend_string * raw = NULL;
    zend_object * frame = NULL;
    zend_long     flags = 0;
    websocket_parser * parser;
    websocket_parser_settings settings;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(raw)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    frame = pion_new_object_arg_0(zend_get_called_scope(execute_data));
    RETVAL_OBJ(frame);
    if(!frame) {
        return;
    }

    websocket_parser_settings_init(&settings);
    settings.on_frame_body = ion_websocket_frame_body;
    parser = emalloc(sizeof(websocket_parser));
    websocket_parser_init(parser);
    websocket_parser_execute(parser, &settings, raw->val, raw->len);

    zend_update_property_long(ion_ce_ION_HTTP_WebSocket_Frame, return_value, STRARGS("flags"), flags);
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, parse, 1)
    METHOD_ARG_STRING(frame, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_HTTP_WebSocket_Frame, factory) {

}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, factory, 1)
    METHOD_ARG_STRING(body, 0)
    METHOD_ARG_LONG(flags, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_HTTP_WebSocket_Frame, build) {

}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, build)

CLASS_METHODS_START(ION_HTTP_WebSocket_Frame)
    METHOD(ION_HTTP_WebSocket_Frame, parse,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_WebSocket_Frame, factory, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_WebSocket_Frame, build,   ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_WebSocket_Frame) {
    pion_register_class(ION_HTTP_WebSocket_Frame, "ION\\HTTP\\WebSocket\\Frame", NULL, CLASS_METHODS(ION_HTTP_WebSocket_Frame));
    pion_init_std_object_handlers(ION_HTTP_WebSocket_Frame);

    PION_CLASS_PROP_LONG(ION_HTTP_WebSocket_Frame, "flags", 0, ZEND_ACC_PUBLIC);
    PION_CLASS_PROP_STRING(ION_HTTP_WebSocket_Frame, "body", "", ZEND_ACC_PUBLIC);

    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_CONT",  0x0);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_TEXT",  0x1);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_BIN",   0x2);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_CLOSE", 0x8);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_PING",  0x9);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_PONG",  0xA);

    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_FLAGS", 0xF);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "MASKED",   WS_FLAG_FIN);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "FIN",      WS_FLAG_MASKED);

    return SUCCESS;
}