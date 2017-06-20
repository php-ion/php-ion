#include "ion_net.h"
#include "ion_init.h"
#include "ion_stream.h"
#include <php_network.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <Zend/zend_exceptions.h>

int pion_net_sock_name(evutil_socket_t sock, short flags, zend_string ** address) {
    socklen_t                 addr_len = sizeof(struct sockaddr);
    struct sockaddr           addr;

    if(flags & PION_NET_NAME_LOCAL) {
        if (getsockname(sock, &addr, &addr_len) == FAILURE) {
//            zend_error(E_NOTICE, "unable to retrieve socket name: %s", strerror(errno));
            return FAILURE;
        }
    } else {
        if (getpeername(sock, &addr, &addr_len) == FAILURE) {
//            zend_error(E_NOTICE, "unable to retrieve peer name: %s", strerror(errno));
            return FAILURE;
        }
    }
    return pion_net_addr_to_name(&addr, addr_len, address);
}

int pion_net_addr_to_name(struct sockaddr * addr, socklen_t addr_len, zend_string ** address) {
    char                    * name = NULL;
    struct sockaddr_in      * sin;
    char	                  addr4[INET_ADDRSTRLEN+1];
#if HAVE_IPV6
    struct sockaddr_in6		* sin6;
    char					  addr6[INET6_ADDRSTRLEN+1];
#endif
    struct sockaddr_un		* s_un;
    char                    * addr_combined;
    int                       type;
    int                       port = 0;
    switch (addr->sa_family) {
        case AF_INET:
            sin = (struct sockaddr_in *) addr;
            evutil_inet_ntop(AF_INET, &sin->sin_addr, addr4, INET_ADDRSTRLEN);
            name = estrdup(addr4);

            port    = htons(sin->sin_port);
            type = PION_NET_NAME_IPV4;
            break;
#if HAVE_IPV6
        case AF_INET6:
            sin6 = (struct sockaddr_in6 *) addr;
            evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, addr6, INET6_ADDRSTRLEN);
            name = estrdup(addr6);
            port    = htons(sin6->sin6_port);
            type = PION_NET_NAME_IPV6;
            break;
#endif
        case AF_UNIX:
            s_un = (struct sockaddr_un *) &addr;
            name = estrdup(s_un->sun_path);
            port = 0;
            type = PION_NET_NAME_UNIX;
            break;
        default:
            name = estrdup("unknown");

            type =  PION_NET_NAME_UNKNOWN;
            break;
    }
    if(port > 0) {
        if(type == PION_NET_NAME_IPV6) {
            spprintf(&addr_combined, 1000, "[%s]:%d", name, port);
        } else {
            spprintf(&addr_combined, 1000, "%s:%d", name, port);
        }
        efree(name);
        *address = zend_string_init(addr_combined, strlen(addr_combined), 0);
        efree(addr_combined);
    } else {
        *address = zend_string_init(name, strlen(name), 0);
        efree(name);
    }
    return type;
}

pion_net_host * pion_net_host_parse(const char * host, size_t host_len) {
    char hostname[MAX_DOMAIN_LENGTH];
    int port = 0;
    if(host_len > MAX_DOMAIN_LENGTH + 2) {
        return NULL;
    }
    if(sscanf(host, "%[^:]:%d", hostname, &port) == 2) {
        pion_net_host * net_host = ecalloc(1, sizeof(pion_net_host));
        net_host->hostname = zend_string_init(hostname, strlen(hostname), 0);
        net_host->port = port;
        return net_host;
    } else {
        return NULL;
    }
}

void pion_net_host_free(pion_net_host * host) {
    zend_string_release(host->hostname);
    efree(host);
}