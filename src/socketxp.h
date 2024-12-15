#ifndef SOCKETXP_H_
#define SOCKETXP_H_

#include <stddef.h>

/*sxp_t, pollsxp_t, addrinfo_t, sockaddr_t*/
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#include "windows/socket_win.h"
#else
#include "unix/socket_unix.h"
#endif

typedef int sxpResult;

enum sxpresults{
  SXP_SUCCESS = 0,
  /*returned when operation whould block on nonblocking socket*/
  SXP_FAILURE_BLOCKING = 1,
  /*returned when a platform-specific error occurred*/
  SXP_ERROR_PLATFORM = -1,
  /*returned when the reason for failure is not known*/
  SXP_ERROR_UNKNOWN = -128
};

enum sxpconfigs {
  SXP_BLOCKING = 0,
  SXP_NONBLOCKING = 1
};

sxpResult sxp_init();
void sxp_cleanup();

sxpResult sxp_create(sxp_t *dst, int family, int type, int protocol, int nonblock);
sxpResult sxp_destroy(sxp_t *sock);

sxpResult sxp_addrinfo_get(addrinfo_t **results, const char *maybe_hostname, const char *serviceport, addrinfo_t *hints);
sxpResult sxp_addrinfo_free(addrinfo_t *info);

/*server-side API*/
sxpResult sxp_bind(sxp_t *sock, sockaddr_t *address, size_t addrlen);
sxpResult sxp_listen(sxp_t *sock);
sxpResult sxp_accept(sxp_t *sock, sxp_t *newsock);

/*client-side API*/
sxpResult sxp_connect(sxp_t *sock, sockaddr_t *address, size_t addrlen);

/*any-side API*/
sxpResult sxp_send(sxp_t *sock, const char *data, size_t size);
sxpResult sxp_recv(sxp_t *sock, char *data, size_t size);

sxpResult sxp_poll(size_t /*maybe NULL*/ *results, pollsxp_t sxps[], size_t sxpcount, int timeout);

#endif /*SOCKETXP_H_*/
