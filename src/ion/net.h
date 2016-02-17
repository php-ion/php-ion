#ifndef PION_NET_H
#define PION_NET_H

#include <php.h>
#include <event2/util.h>

#define MAX_DOMAIN_LENGTH 255

#define PION_NET_NAME_UNKNOWN 0
#define PION_NET_NAME_IPV4    1
#define PION_NET_NAME_IPV6    2
#define PION_NET_NAME_UNIX    3

#define PION_NET_NAME_REMOTE     0
#define PION_NET_NAME_LOCAL      1

#ifdef WIN32
#define LOCAL_SOCKETPAIR_AF AF_INET
#else
#define LOCAL_SOCKETPAIR_AF AF_UNIX
#endif

int pion_net_sock_name(evutil_socket_t sock, short flags, zend_string ** addr);
int pion_net_addr_to_name(struct sockaddr * addr, socklen_t addr_len, zend_string ** address);

typedef struct _pion_net_host {
    zend_string * hostname;
    zend_long     port;
} pion_net_host;

pion_net_host * pion_net_host_parse(const char * host, size_t host_len);
void pion_net_host_free(pion_net_host * host);

#endif //PION_NET_H
