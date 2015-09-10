#include "net.h"
#include <arpa/inet.h>
#include <sys/un.h>

int _pion_net_sock_name(int sock, short flags, char ** address, int * port TSRMLS_DC) {
    socklen_t                 addr_len;
    struct sockaddr_storage   addr;
    struct sockaddr_in      * sin;
    char	                  addr4[INET_ADDRSTRLEN+1];
#if HAVE_IPV6
    struct sockaddr_in6		* sin6;
    char					  addr6[INET6_ADDRSTRLEN+1];
#endif
    struct sockaddr_un		* s_un;

    if(flags == PION_NET_NAME_REMOTE) {
        if (getpeername(sock, (struct sockaddr*)&addr, &addr_len) < 0) {
            return FAILURE;
        }
    } else {
        if (getsockname(sock, (struct sockaddr*)&addr, &addr_len) < 0) {
            return FAILURE;
        }
    }

    switch (addr.ss_family) {
        case AF_INET:
            sin = (struct sockaddr_in *) &addr;
            inet_ntop(AF_INET, &sin->sin_addr, addr4, INET_ADDRSTRLEN);
            *address = estrdup(addr4);
            *port    = htons(sin->sin_port);
            return PION_NET_NAME_IPV4;
#if HAVE_IPV6
        case AF_INET6:
            sin6 = (struct sockaddr_in6 *) &addr;
            inet_ntop(AF_INET6, &sin6->sin6_addr, addr6, INET6_ADDRSTRLEN);
            *address = estrdup(addr6);
            *port    = htons(sin6->sin6_port);
            return PION_NET_NAME_IPV6;
#endif
        case AF_UNIX:
            s_un = (struct sockaddr_un *) &addr;
            *address = estrdup(s_un->sun_path);
            return PION_NET_NAME_UNIX;
        default:
            *address = estrdup("unknown");
            zend_error(E_NOTICE, "Unsupported address family %d", addr.ss_family);
            return PION_NET_NAME_UNKNOWN;
    }
}
