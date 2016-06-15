#include <ion/http.h>
#include "ion.h"

zend_object_handlers ion_oh_ION_HTTP_WebSocketParser;
zend_class_entry * ion_ce_ION_HTTP_WebSocketParser;

int ion_http_ws_message_begin(websocket_parser * p) {
    zval zframe;
    ion_http_websocket_frame * frame;
    ion_http_body_parser * parser = p->data;

    object_init_ex(&zframe, ion_ce_ION_HTTP_WebSocket_Frame);
    frame = get_instance(&zframe, ion_http_websocket_frame);
    frame->flags = p->flags;
    frame->body = zend_string_alloc(p->length, 0);
    memcpy(frame->mask, p->mask, 4);
    ZSTR_VAL(frame->body)[p->length] = '\000';

    parser->parser.websocket->frame = frame;
    return 0;
}

int ion_http_ws_message_body(websocket_parser * p, const char * at, size_t length) {
    ion_http_body_parser * parser = p->data;
    ZEND_ASSERT(parser->parser.websocket->frame);
    ion_http_websocket_frame * frame = parser->parser.websocket->frame;

    if(websocket_parser_has_mask(p)) {
        websocket_parser_decode(&frame->body->val[p->offset], at, length, p);
    } else {
        memcpy(&frame->body->val[p->offset], at, length);
    }

    return 0;
}

int ion_http_ws_message_end(websocket_parser * p) {
    ion_http_body_parser * parser = p->data;
    ZEND_ASSERT(parser->parser.websocket->frame);
    if(parser->parser.websocket->on_frame) {
        zval zframe;
        ZVAL_OBJ(&zframe, ION_OBJ(parser->parser.websocket->frame));
        ion_promisor_sequence_invoke(parser->parser.websocket->on_frame, &zframe);
    }
    zend_object_release(ION_OBJ(parser->parser.websocket->frame));
    parser->parser.websocket->frame = NULL;
    parser->parser.websocket->frames_num++;
    return 0;
}

zend_object * ion_http_websocket_parser_init(zend_class_entry * ce) {
    ion_http_body_parser * parser = ecalloc(1, sizeof(ion_http_body_parser));
    parser->type = ion_http_type_websocket;
    parser->settings.websocket = ecalloc(1, sizeof(websocket_parser_settings));
    parser->settings.websocket->on_frame_header = ion_http_ws_message_begin;
    parser->settings.websocket->on_frame_body   = ion_http_ws_message_body;
    parser->settings.websocket->on_frame_end    = ion_http_ws_message_end;
    parser->parser.websocket = ecalloc(1, sizeof(ion_websocket_parser));
    websocket_parser_init(&parser->parser.websocket->p);
    parser->parser.websocket->p.data = parser;
    RETURN_INSTANCE(ION_HTTP_WebSocketParser, parser);
}

void ion_http_websocket_parser_free(zend_object * obj) {
    ion_http_body_parser * parser = get_object_instance(obj, ion_http_body_parser);
    if(parser->parser.websocket->on_frame) {
        zend_object_release(parser->parser.websocket->on_frame);
    }
    efree(parser->settings.websocket);
    efree(parser->parser.websocket);
}


/** public function ION\HTTP\WebSocketParser::frame() : ION\Sequence */
CLASS_METHOD(ION_HTTP_WebSocketParser, frame) {
    ion_http_body_parser * parser = get_this_instance(ion_http_body_parser);

    if(!parser->parser.websocket->on_frame) {
        parser->parser.websocket->on_frame = ion_promisor_sequence_new(NULL);
        ion_promisor_store(parser->parser.websocket->on_frame, parser);
    }

    zend_object_addref(parser->parser.websocket->on_frame);
    RETURN_OBJ(parser->parser.websocket->on_frame);
}


METHOD_WITHOUT_ARGS(ION_HTTP_WebSocketParser, frame)

/** public function ION\HTTP\WebSocketParser::getParsedCount() : int */
CLASS_METHOD(ION_HTTP_WebSocketParser, getParsedCount) {
    ion_http_body_parser * parser = get_this_instance(ion_http_body_parser);

    RETURN_LONG(parser->parser.websocket->frames_num);
}


METHOD_WITHOUT_ARGS(ION_HTTP_WebSocketParser, getParsedCount)

/** public function ION\HTTP\WebSocketParser::hasUnparsedFrame() : bool */
CLASS_METHOD(ION_HTTP_WebSocketParser, hasUnparsedFrame) {
    ion_http_body_parser * parser = get_this_instance(ion_http_body_parser);

    if(parser->parser.websocket->frame) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocketParser, hasUnparsedFrame)


/** public function ION\HTTP\WebSocketParser::__invoke() : mixed */
CLASS_METHOD(ION_HTTP_WebSocketParser, __invoke) {
    ion_http_body_parser * parser = get_this_instance(ion_http_body_parser);
    zend_string          * raw = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(raw)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    size_t nparsed = websocket_parser_execute(&parser->parser.websocket->p, parser->settings.websocket, ZSTR_VAL(raw), ZSTR_LEN(raw));
    ZEND_ASSERT(nparsed == ZSTR_LEN(raw));
    if(parser->parser.websocket->p.require) {
        RETURN_LONG(parser->parser.websocket->p.require);
    } else {
        RETURN_LONG(-1);
    }
}


METHOD_ARGS_BEGIN(ION_HTTP_WebSocketParser, __invoke, 1)
    METHOD_ARG_STRING(frame, 0)
METHOD_ARGS_END();



CLASS_METHODS_START(ION_HTTP_WebSocketParser)
    METHOD(ION_HTTP_WebSocketParser, frame,            ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocketParser, getParsedCount,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocketParser, hasUnparsedFrame, ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocketParser, __invoke,         ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_WebSocketParser) {
    pion_register_class(ION_HTTP_WebSocketParser, "ION\\HTTP\\WebSocketParser", ion_http_websocket_parser_init, CLASS_METHODS(ION_HTTP_WebSocketParser));
    pion_init_std_object_handlers(ION_HTTP_WebSocketParser);

    PION_CLASS_CONST_LONG(ION_HTTP_WebSocketParser, "UUID",  WEBSOCKET_UUID);

    pion_set_object_handler(ION_HTTP_WebSocketParser, free_obj, ion_http_websocket_parser_free);
    pion_set_object_handler(ION_HTTP_WebSocketParser, clone_obj, NULL);

    return SUCCESS;
}

