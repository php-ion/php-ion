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
    char                    * addr_combined;
    int                       type;

    if(flags & PION_NET_NAME_LOCAL) {
        if (getsockname(sock, (struct sockaddr*)&addr, &addr_len) < 0) {
            return FAILURE;
        }
    } else {
        if (getpeername(sock, (struct sockaddr*)&addr, &addr_len) < 0) {
            return FAILURE;
        }
    }

    switch (addr.ss_family) {
        case AF_INET:
            sin = (struct sockaddr_in *) &addr;
            inet_ntop(AF_INET, &sin->sin_addr, addr4, INET_ADDRSTRLEN);
            *address = estrdup(addr4);
            *port    = htons(sin->sin_port);
            type = PION_NET_NAME_IPV4;
            break;
#if HAVE_IPV6
        case AF_INET6:
            sin6 = (struct sockaddr_in6 *) &addr;
            inet_ntop(AF_INET6, &sin6->sin6_addr, addr6, INET6_ADDRSTRLEN);
            *address = estrdup(addr6);
            *port    = htons(sin6->sin6_port);
            type = PION_NET_NAME_IPV6;
            break;
#endif
        case AF_UNIX:
            s_un = (struct sockaddr_un *) &addr;
            *address = estrdup(s_un->sun_path);
            *port = 0;
            type = PION_NET_NAME_UNIX;
            break;
        default:
            *address = estrdup("unknown");
            zend_error(E_NOTICE, "Unsupported address family %d", addr.ss_family);
            *port = 0;
            type =  PION_NET_NAME_UNKNOWN;
            break;
    }
    if((flags & PION_NET_NAME_AS_STRING) && *port > 0) {
        if(type == PION_NET_NAME_IPV6) {
            spprintf(&addr_combined, 1000, "[%s]:%d", *address, *port);
        } else {
            spprintf(&addr_combined, 1000, "%s:%d", *address, *port);
        }
        efree(*address);
        *address = estrdup(addr_combined);
        efree(addr_combined);
    }
    return type;
}
