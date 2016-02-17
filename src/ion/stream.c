#include "stream.h"
#include "ion.h"

zend_bool ion_buffer_pair(ion_buffer ** one, ion_buffer ** two) {
    evutil_socket_t  socks[2] = {-1, -1};
    ion_buffer     * pair[2];

    if(evutil_socketpair(LOCAL_SOCKETPAIR_AF, SOCK_STREAM, 0, socks) == FAILURE) {
        return false;
    }

    pair[0] = bufferevent_socket_new(GION(base), socks[0], BEV_OPT_CLOSE_ON_FREE);
    pair[1] = bufferevent_socket_new(GION(base), socks[1], BEV_OPT_CLOSE_ON_FREE);

    if(bufferevent_enable(pair[0], EV_READ | EV_WRITE) == FAILURE ||
       bufferevent_enable(pair[1], EV_READ | EV_WRITE) == FAILURE) {
        bufferevent_free(pair[0]);
        bufferevent_free(pair[1]);
        zend_throw_exception(ion_ce_ION_StreamException, "Failed to enable stream", 0);
        return false;
    }

    *one = pair[0];
    *two = pair[1];

    return true;
}
