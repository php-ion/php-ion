#ifndef PION_NET_H
#define PION_NET_H

#include <php.h>

#define PION_NET_NAME_UNKNOWN 0
#define PION_NET_NAME_IPV4    1
#define PION_NET_NAME_IPV6    2
#define PION_NET_NAME_UNIX    3

#define PION_NET_NAME_REMOTE     0
#define PION_NET_NAME_LOCAL      1
#define PION_NET_NAME_AS_STRING  2

#define pion_net_sock_name(sock_fd, flags, addr_p, port_p) _pion_net_sock_name(sock_fd, flags, addr_p, port_p TSRMLS_CC)

int _pion_net_sock_name(int sock, short flags, char ** addr, int * port TSRMLS_DC);

#endif //PION_NET_H
