#ifndef SOCKETXP_H_
#define SOCKETXP_H_

#include <stddef.h>

/*sxp_t, pollsxp_t, addrinfo_t, sockaddr_t*/
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)

#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET sxp_t;
typedef WSAPOLLFD pollsxp_t;
typedef struct addrinfo addrinfo_t;
typedef struct sockaddr sockaddr_t;

#define SXP_POLLIN POLLRDNORM
#define SXP_POLLOUT POLLWRNORM

#else

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200112L
#define _POSIX_C_SOURCE 200112L
#define _POSIX_SOURCE 1
#endif /*_POSIX_C_SOURCE*/

#include <netdb.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef int sxp_t;
typedef struct pollfd pollsxp_t;
typedef struct addrinfo addrinfo_t;
typedef struct sockaddr sockaddr_t;

#define SXP_POLLIN POLLIN
#define SXP_POLLOUT POLLOUT

#endif

typedef int sxpResult;

enum sxpresults {
    SXP_SUCCESS = 0,
    /*returned when operation whould block on nonblocking socket*/
    SXP_TRY_AGAIN = 1,
    /*returned when packet size is too large*/
    SXP_TOO_BIG = 2,
    /*returned when operation will complete asynchronously*/
    SXP_ASYNC = 3,
    /*returned when a platform-specific error occurred*/
    SXP_ERROR_PLATFORM = -1,
    /*returned when out of memory*/
    SXP_ERROR_MEMORY = -2,
    /*returned when one or multiple arguments are invalid*/
    SXP_ERROR_INVAL = -3,
    /*returned when a io or connection error occured*/
    SXP_ERROR_IO = -4,
    /*returned when the socket is/has been closed */
    SXP_ERROR_CLOSED = -5,
    /*returned when the reason for failure is not known*/
    SXP_ERROR_UNKNOWN = -128
};

enum sxpconfigs { SXP_BLOCKING = 0, SXP_NONBLOCKING = 1 };

sxpResult sxp_init();
sxpResult sxp_cleanup();

sxpResult sxp_create(sxp_t *dst, int family, int type, int protocol);
sxpResult sxp_destroy(sxp_t *sock);

sxpResult sxp_nbio_set(sxp_t *sock, int nonblockingio);

sxpResult sxp_addrinfo_get(addrinfo_t **results, const char *maybe_hostname,
                           const char *serviceport, addrinfo_t *hints);
sxpResult sxp_addrinfo_free(addrinfo_t *info);

/*server-side API*/
sxpResult sxp_bind(sxp_t *sock, const sockaddr_t *address, size_t addrlen);
sxpResult sxp_listen(sxp_t *sock, size_t backlog);
sxpResult sxp_accept(sxp_t *sock, sxp_t *newsock);

/*client-side API*/
sxpResult sxp_connect(sxp_t *sock, sockaddr_t *address, size_t addrlen);

/*any-side API*/
sxpResult sxp_send(sxp_t *sock, const char *data, size_t size);
sxpResult sxp_recv(sxp_t *sock, char *data, size_t *num_read, size_t size);

sxpResult sxp_poll(size_t /*maybe NULL*/ *results, pollsxp_t sxps[],
                   size_t sxpcount, int timeout);

#endif /*SOCKETXP_H_*/
