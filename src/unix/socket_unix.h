#ifndef SOCKET_UNIX_H_
#define SOCKET_UNIX_H_

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif /*_POSIX_C_SOURCE*/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netdb.h>

/*sxp_t, pollsxp_t, addrinfo_t, sockaddr_t*/

typedef int sxp_t;
typedef struct pollfd pollsxp_t;
typedef struct addrinfo addrinfo_t;
typedef struct sockaddr sockaddr_t;

#endif /*SOCKET_UNIX_H_*/
