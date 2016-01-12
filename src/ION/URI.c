#include <php.h>
#include "../pion.h"
#include <ext/standard/url.h>

zend_object_handlers ion_oh_ION_URI;
zend_class_entry * ion_ce_ION_URI;


zend_object * ion_uri_init(zend_class_entry * ce) {
    ion_uri * theuri = ecalloc(1, sizeof(ion_uri));
    RETURN_INSTANCE(ION_URI, theuri);
}

void ion_uri_free(zend_object * object) {
    ion_uri * theuri = get_object_instance(object, ion_uri);
    zend_object_std_dtor(object);
    if(theuri->scheme) {
        zend_string_release(theuri->scheme);
    }
    if(theuri->user) {
        zend_string_release(theuri->user);
    }
    if(theuri->pass) {
        zend_string_release(theuri->pass);
    }
    if(theuri->host) {
        zend_string_release(theuri->host);
    }
    if(theuri->path) {
        zend_string_release(theuri->path);
    }
    if(theuri->query) {
        zend_string_release(theuri->query);
    }
    if(theuri->fragment) {
        zend_string_release(theuri->fragment);
    }
}

zend_string * ion_uri_stringify(zend_object * uri, unsigned short parts) {
    ion_uri     * theuri = get_object_instance(uri, ion_uri);
    zend_string * buf;
    size_t        length = 0;
    size_t        pos = 0;

    if(theuri->scheme && (parts & URI_SCHEME)) {
        length += theuri->scheme->len + 3; // scheme + "://"
    }
    if(theuri->user && (parts & URI_USER)) {
        length += theuri->user->len + 1;  // user + "@"
        if(theuri->pass && (parts & URI_PASS)) {
            length += theuri->pass->len + 1; // ":" + pass
        }
    }
    if(theuri->host && (parts & URI_HOST)) {
        length += theuri->host->len;
    }
    if(theuri->port && (parts & URI_PORT)) {
        length++;                                   // ":" + port
        if(theuri->port >= 10000) {
            length += 5;
        } else if(theuri->port >= 1000) {
            length += 4;
        } else if(theuri->port >= 100) {
            length += 3;
        } else if(theuri->port >= 10) {
            length += 2;
        } else {
            length += 1;
        }
    }
    if(theuri->path && (parts & URI_PATH)) {
        length += theuri->path->len;
    }
    if(theuri->query && (parts & URI_QUERY)) {
        length += theuri->query->len + 1; // "?" + query
    }
    if(theuri->fragment  && (parts & URI_FRAGMENT)) {
        length += theuri->fragment->len + 1; // "#" + fragment
    }

    buf = zend_string_alloc(length, 0);

    if(theuri->scheme && (parts & URI_SCHEME)) {
        memcpy(&buf->val[pos], theuri->scheme->val, theuri->scheme->len);
        pos += theuri->scheme->len;
        memcpy(&buf->val[pos], "://", sizeof("://") - 1);
        pos += 3;
    }
    if(theuri->user && (parts & URI_USER)) {
        memcpy(&buf->val[pos], theuri->user->val, theuri->user->len);
        pos += theuri->user->len;
        if(theuri->pass && (parts & URI_PASS)) {
            buf->val[pos] = ':';
            pos++;
            memcpy(&buf->val[pos], theuri->pass->val, theuri->pass->len);
            pos += theuri->pass->len;
        }
        buf->val[pos] = '@';
        pos++;
    }
    if(theuri->host && (parts & URI_HOST)) {
        memcpy(&buf->val[pos], theuri->host->val, theuri->host->len);
        pos += theuri->host->len;
    }
    if(theuri->port && (parts & URI_PORT)) {
        pos += sprintf(&buf->val[pos], ":%d", theuri->port);
    }
    if(theuri->path && (parts & URI_PATH)) {
        memcpy(&buf->val[pos], theuri->path->val, theuri->path->len);
        pos += theuri->path->len;
    }
    if(theuri->query && (parts & URI_QUERY)) {
        buf->val[pos] = '?';
        pos++;
        memcpy(&buf->val[pos], theuri->query->val, theuri->query->len);
        pos += theuri->query->len;
    }
    if(theuri->fragment && (parts & URI_FRAGMENT)) {
        buf->val[pos] = '#';
        pos++;
        memcpy(&buf->val[pos], theuri->fragment->val, theuri->fragment->len);
        pos += theuri->fragment->len;
    }
    ZEND_ASSERT(length == pos);
    buf->val[length] = '\0';

    return buf;
}

/**
 * uri_string may be NULL
 */
zend_object * ion_uri_parse(zend_string * uri_string) {
    zend_object * uri_object = NULL;
    ion_uri     * uri = NULL;
    php_url     * parsed_url = NULL;

    if(uri_string) {
        parsed_url = php_url_parse_ex(uri_string->val, uri_string->len);
        if(!parsed_url) {
            zend_throw_exception(ion_ce_InvalidArgumentException, "URI could not be parsed", 0);
            return NULL;
        }
    }
    uri_object = pion_new_object_arg_0(ion_ce_ION_URI);
    uri = get_object_instance(uri_object, ion_uri);
    if(uri_string) {
        if(parsed_url->scheme) {
            uri->scheme = zend_string_init(parsed_url->scheme, strlen(parsed_url->scheme), 0);
        }
        if(parsed_url->user) {
            uri->user = zend_string_init(parsed_url->user, strlen(parsed_url->user), 0);
        }
        if(parsed_url->pass) {
            uri->pass = zend_string_init(parsed_url->pass, strlen(parsed_url->pass), 0);
        }
        if(parsed_url->user) {
            uri->host = zend_string_init(parsed_url->host, strlen(parsed_url->host), 0);
        }
        if(parsed_url->port) {
            uri->port = parsed_url->port;
        }
        if(parsed_url->path) {
            uri->path = zend_string_init(parsed_url->path, strlen(parsed_url->path), 0);
        }
        if(parsed_url->query) {
            uri->query = zend_string_init(parsed_url->query, strlen(parsed_url->query), 0);
        }
        if(parsed_url->fragment) {
            uri->fragment = zend_string_init(parsed_url->fragment, strlen(parsed_url->fragment), 0);
        }

        php_url_free(parsed_url);
    }

    return uri_object;
}

zend_object * ion_uri_clone(zend_object * proto_obj) {
    ion_uri * proto = get_object_instance(proto_obj, ion_uri);
    ion_uri * clone = ecalloc(1, sizeof(ion_uri));

    zend_object_std_init(&clone->std, proto->std.ce);
    object_properties_init(&clone->std, proto->std.ce);
    clone->std.handlers = proto->std.handlers;
    zend_object * clone_obj = &clone->std;
    zend_objects_clone_members(clone_obj, proto_obj);

    if(proto->scheme) {
        clone->scheme = zend_string_copy(proto->scheme);
    }
    if(proto->user) {
        clone->user = zend_string_copy(proto->user);
    }
    if(proto->pass) {
        clone->pass = zend_string_copy(proto->pass);
    }
    if(proto->host) {
        clone->host = zend_string_copy(proto->host);
    }
    if(proto->port) {
        clone->port = proto->port;
    }
    if(proto->path) {
        clone->path = zend_string_copy(proto->path);
    }
    if(proto->query) {
        clone->query = zend_string_copy(proto->query);
    }
    if(proto->fragment) {
        clone->fragment = zend_string_copy(proto->fragment);
    }

    return clone_obj;
}

zend_object * ion_uri_clone_handler(zval * object) {
    return ion_uri_clone(Z_OBJ_P(object));
}

CLASS_METHOD(ION_URI, parse) {
    zend_object * uri_object = NULL;
    zend_string * uri_string = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(uri_string)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    uri_object = ion_uri_parse(uri_string);
    if(uri_object) {
        RETURN_OBJ(uri_object);
    }
}

METHOD_ARGS_BEGIN(ION_URI, parse, 1)
    METHOD_ARG_STRING(uri, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_URI, hasScheme) {
    ion_uri * uri = get_this_instance(ion_uri);
    if(uri->scheme) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE
    }
}

METHOD_WITHOUT_ARGS(ION_URI, hasScheme);

CLASS_METHOD(ION_URI, getScheme) {
    ion_uri * uri = get_this_instance(ion_uri);
    if(uri->scheme) {
        RETURN_STR(zend_string_copy(uri->scheme));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getScheme);

CLASS_METHOD(ION_URI, withScheme) {
    zend_string * scheme = NULL;
    zend_object * uri = NULL;
    ion_uri     * theuri = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(scheme)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    uri = ion_uri_clone(Z_OBJ_P(getThis()));
    theuri = get_object_instance(uri, ion_uri);

    if(theuri->scheme) {
        zend_string_release(theuri->scheme);
    }
    theuri->scheme = zend_string_copy(scheme);

    RETURN_OBJ(uri);
}

METHOD_ARGS_BEGIN(ION_URI, withScheme, 1)
    METHOD_ARG_STRING(scheme, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_URI, getAuthority) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->host) {
        RETURN_STR(ion_uri_stringify(Z_OBJ_P(getThis()), URI_USER | URI_PASS | URI_HOST | URI_PORT));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getAuthority);

CLASS_METHOD(ION_URI, hasUserInfo) {
    ion_uri * uri = get_this_instance(ion_uri);
    if(uri->user) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE
    }
}

METHOD_WITHOUT_ARGS(ION_URI, hasUserInfo);


CLASS_METHOD(ION_URI, getUserInfo) {
    ion_uri     * theuri = get_this_instance(ion_uri);
    zend_string * buff = NULL;
    if(theuri->user) {
        if(theuri->pass) {
            buff = strpprintf(1024, "%s:%s", theuri->user->val, theuri->pass->val);
            if(buff) {
                RETURN_STR(buff);
            } else {
                zend_throw_exception(ion_ce_RuntimeException, "Failed to generate user info", 0);
                return;
            }
        } else {
            RETURN_STR(zend_string_copy(theuri->user));
        }
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getUserInfo);

CLASS_METHOD(ION_URI, getUserName) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->user) {
        RETURN_STR(zend_string_copy(theuri->user));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getUserName);

CLASS_METHOD(ION_URI, hasUserPassword) {
    ion_uri * uri = get_this_instance(ion_uri);
    if(uri->pass) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE
    }
}

METHOD_WITHOUT_ARGS(ION_URI, hasUserPassword);

CLASS_METHOD(ION_URI, getUserPassword) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->pass) {
        RETURN_STR(zend_string_copy(theuri->pass));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getUserPassword);

CLASS_METHOD(ION_URI, withUserInfo) {
    zend_string * user = NULL;
    zend_string * password = NULL;
    zend_object * uri = NULL;
    ion_uri     * theuri = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(user)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(password)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    uri = ion_uri_clone(Z_OBJ_P(getThis()));
    theuri = get_object_instance(uri, ion_uri);

    if(theuri->user) {
        zend_string_release(theuri->user);
    }
    theuri->user = zend_string_copy(user);
    if(password) {
        if(theuri->pass) {
            zend_string_release(theuri->pass);
        }
        theuri->pass = zend_string_copy(password);
    }


    RETURN_OBJ(uri);
}

METHOD_ARGS_BEGIN(ION_URI, withUserInfo, 1)
    METHOD_ARG_STRING(user, 0)
    METHOD_ARG_STRING(password, 0)
METHOD_ARGS_END();


CLASS_METHOD(ION_URI, hasHost) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->host) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE
    }
}

METHOD_WITHOUT_ARGS(ION_URI, hasHost);

CLASS_METHOD(ION_URI, getHost) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->host) {
        RETURN_STR(zend_string_copy(theuri->host));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getHost);

CLASS_METHOD(ION_URI, withHost) {
    zend_string * host = NULL;
    zend_object * uri = NULL;
    ion_uri     * theuri = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(host)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    uri = ion_uri_clone(Z_OBJ_P(getThis()));
    theuri = get_object_instance(uri, ion_uri);

    if(theuri->host) {
        zend_string_release(theuri->host);
    }
    theuri->host = zend_string_copy(host);

    RETURN_OBJ(uri);
}

METHOD_ARGS_BEGIN(ION_URI, withHost, 1)
    METHOD_ARG_STRING(host, 0)
METHOD_ARGS_END();


CLASS_METHOD(ION_URI, hasPort) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->port) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE
    }
}

METHOD_WITHOUT_ARGS(ION_URI, hasPort);

CLASS_METHOD(ION_URI, getPort) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->port) {
        RETURN_LONG(theuri->port);
    } else {
        RETURN_NULL();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getPort);

CLASS_METHOD(ION_URI, withPort) {
    zend_long     port = 0;
    zend_object * uri = NULL;
    ion_uri     * theuri = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(port)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(port > 0xFFFF) {
        zend_throw_exception(ion_ce_ION_InvalidUsageException, "Port size should be between 0 and 65535", 0);
        return;
    }

    uri = ion_uri_clone(Z_OBJ_P(getThis()));
    theuri = get_object_instance(uri, ion_uri);

    theuri->port = (unsigned short)port;

    RETURN_OBJ(uri);
}

METHOD_ARGS_BEGIN(ION_URI, withPort, 1)
    METHOD_ARG_LONG(port, 0)
METHOD_ARGS_END();


CLASS_METHOD(ION_URI, hasPath) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->path) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE
    }
}

METHOD_WITHOUT_ARGS(ION_URI, hasPath);

CLASS_METHOD(ION_URI, getPath) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->path) {
        RETURN_STR(zend_string_copy(theuri->path));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getPath);

CLASS_METHOD(ION_URI, withPath) {
    zend_string * path = NULL;
    zend_object * uri = NULL;
    ion_uri     * theuri = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(path)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    uri = ion_uri_clone(Z_OBJ_P(getThis()));
    theuri = get_object_instance(uri, ion_uri);

    if(theuri->path) {
        zend_string_release(theuri->path);
    }
    theuri->path = zend_string_copy(path);

    RETURN_OBJ(uri);
}

METHOD_ARGS_BEGIN(ION_URI, withPath, 1)
    METHOD_ARG_STRING(path, 0)
METHOD_ARGS_END();


CLASS_METHOD(ION_URI, hasQuery) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->query) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE
    }
}

METHOD_WITHOUT_ARGS(ION_URI, hasQuery);

CLASS_METHOD(ION_URI, getQuery) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->query) {
        RETURN_STR(zend_string_copy(theuri->query));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getQuery);

CLASS_METHOD(ION_URI, withQuery) {
    zend_string * query = NULL;
    zend_object * uri = NULL;
    ion_uri     * theuri = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(query)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    uri = ion_uri_clone(Z_OBJ_P(getThis()));
    theuri = get_object_instance(uri, ion_uri);

    if(theuri->query) {
        zend_string_release(theuri->query);
    }
    theuri->query = zend_string_copy(query);

    RETURN_OBJ(uri);
}

METHOD_ARGS_BEGIN(ION_URI, withQuery, 1)
    METHOD_ARG_STRING(query, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_URI, hasFragment) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->fragment) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE
    }
}

METHOD_WITHOUT_ARGS(ION_URI, hasFragment);

CLASS_METHOD(ION_URI, getFragment) {
    ion_uri * theuri = get_this_instance(ion_uri);
    if(theuri->fragment) {
        RETURN_STR(zend_string_copy(theuri->fragment));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_URI, getFragment);

CLASS_METHOD(ION_URI, withFragment) {
    zend_string * fragment = NULL;
    zend_object * uri = NULL;
    ion_uri     * theuri = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(fragment)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    uri = ion_uri_clone(Z_OBJ_P(getThis()));
    theuri = get_object_instance(uri, ion_uri);

    if(theuri->fragment) {
        zend_string_release(theuri->fragment);
    }
    theuri->fragment = zend_string_copy(fragment);

    RETURN_OBJ(uri);
}

METHOD_ARGS_BEGIN(ION_URI, withFragment, 1)
    METHOD_ARG_STRING(fragment, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_URI, __toString) {
    RETURN_STR(ion_uri_stringify(Z_OBJ_P(getThis()), URI_ALL));
}

METHOD_WITHOUT_ARGS(ION_URI, __toString);

CLASS_METHODS_START(ION_URI)
    METHOD(ION_URI, parse,           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

    METHOD(ION_URI, hasScheme,       ZEND_ACC_PUBLIC)
    METHOD(ION_URI, getScheme,       ZEND_ACC_PUBLIC)
    METHOD(ION_URI, withScheme,      ZEND_ACC_PUBLIC)

    METHOD(ION_URI, getAuthority,    ZEND_ACC_PUBLIC)

    METHOD(ION_URI, hasUserInfo,     ZEND_ACC_PUBLIC)
    METHOD(ION_URI, getUserInfo,     ZEND_ACC_PUBLIC)
    METHOD(ION_URI, getUserName,     ZEND_ACC_PUBLIC)
    METHOD(ION_URI, getUserPassword, ZEND_ACC_PUBLIC)
    METHOD(ION_URI, hasUserPassword, ZEND_ACC_PUBLIC)
    METHOD(ION_URI, withUserInfo,    ZEND_ACC_PUBLIC)

    METHOD(ION_URI, hasHost,         ZEND_ACC_PUBLIC)
    METHOD(ION_URI, getHost,         ZEND_ACC_PUBLIC)
    METHOD(ION_URI, withHost,        ZEND_ACC_PUBLIC)

    METHOD(ION_URI, hasPort,         ZEND_ACC_PUBLIC)
    METHOD(ION_URI, getPort,         ZEND_ACC_PUBLIC)
    METHOD(ION_URI, withPort,        ZEND_ACC_PUBLIC)

    METHOD(ION_URI, hasPath,         ZEND_ACC_PUBLIC)
    METHOD(ION_URI, getPath,         ZEND_ACC_PUBLIC)
    METHOD(ION_URI, withPath,        ZEND_ACC_PUBLIC)

    METHOD(ION_URI, hasQuery,        ZEND_ACC_PUBLIC)
    METHOD(ION_URI, getQuery,        ZEND_ACC_PUBLIC)
    METHOD(ION_URI, withQuery,       ZEND_ACC_PUBLIC)

    METHOD(ION_URI, hasFragment,     ZEND_ACC_PUBLIC)
    METHOD(ION_URI, getFragment,     ZEND_ACC_PUBLIC)
    METHOD(ION_URI, withFragment,    ZEND_ACC_PUBLIC)

    METHOD(ION_URI, __toString,      ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_URI) {
    pion_register_class(ION_URI, "ION\\URI", ion_uri_init, CLASS_METHODS(ION_URI));
    pion_init_std_object_handlers(ION_URI);
    pion_set_object_handler(ION_URI, free_obj, ion_uri_free);
    pion_set_object_handler(ION_URI, clone_obj, ion_uri_clone_handler);

    return SUCCESS;
}