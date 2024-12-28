#include "socketxp.h"
#include <errno.h>
#include <fcntl.h>

static sxpResult sxp_map_eai_error(int error, int system_errno);
static sxpResult sxp_map_error(int error);

sxpResult sxp_init() { return SXP_SUCCESS; }
sxpResult sxp_cleanup() { return SXP_SUCCESS; }

sxpResult sxp_create(sxp_t *dst, int family, int type, int protocol, int nonblock)
{
  int sock;
  if (!dst) return SXP_ERROR_INVAL;
  if ((sock = socket(family, type, protocol)) < 0) { return sxp_map_error(errno); }
  if (nonblock == SXP_NONBLOCKING) {
    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
      close(sock);
      return SXP_ERROR_UNKNOWN;
    }
  } else if (nonblock != SXP_BLOCKING) {
    close(sock);
    return SXP_ERROR_INVAL;
  }

  *dst = sock;
  return SXP_SUCCESS;
}

sxpResult sxp_destroy(sxp_t *sock)
{
  if (!sock) return SXP_ERROR_INVAL;
  if (close(*sock) < 0) return sxp_map_error(errno);
  return SXP_SUCCESS;
}

sxpResult sxp_addrinfo_get(addrinfo_t **results, const char *maybe_hostname,
                           const char *serviceport, addrinfo_t *hints)
{
  int result = getaddrinfo(maybe_hostname, serviceport, hints, results);
  if (result != 0) { return sxp_map_eai_error(result, errno); }
  return SXP_SUCCESS;
}

sxpResult sxp_addrinfo_free(addrinfo_t *info) { freeaddrinfo(info); return SXP_SUCCESS; }

/*server-side API*/
sxpResult sxp_bind(sxp_t *sock, const sockaddr_t *address, size_t addrlen)
{
  if (!sock) return SXP_ERROR_INVAL;
  if (bind(*sock, address, addrlen) < 0) return sxp_map_error(errno);
  return SXP_SUCCESS;
}

sxpResult sxp_listen(sxp_t *sock, size_t backlog)
{
  if (!sock) return SXP_ERROR_INVAL;
  if (listen(*sock, backlog) < 0) return sxp_map_error(errno);
  return SXP_SUCCESS;
}

sxpResult sxp_accept(sxp_t *sock, sxp_t *newsock)
{
  int socket;
  if (!sock || !newsock) return SXP_ERROR_INVAL;
  socket = accept(*sock, NULL, NULL);
  if (socket < 0) return sxp_map_error(errno);
  *newsock = socket;
  return SXP_SUCCESS;
}

/*client-side API*/
sxpResult sxp_connect(sxp_t *sock, sockaddr_t *address, size_t addrlen)
{
  if (!sock) return SXP_ERROR_INVAL;
  if (connect(*sock, address, addrlen) < 0) return sxp_map_error(errno);
  return SXP_SUCCESS;
}

/*any-side API*/
sxpResult sxp_send(sxp_t *sock, const char *data, size_t size)
{
  if (!sock) return SXP_ERROR_INVAL;
  if (send(*sock, data, size, MSG_NOSIGNAL) < 0) return sxp_map_error(errno);
  return SXP_SUCCESS;
}

sxpResult sxp_recv(sxp_t *sock, char *data, size_t *num_read, size_t size)
{
  ssize_t read;
  if (!sock || !data || !num_read) return SXP_ERROR_INVAL;
  if ((read = recv(*sock, data, size, 0)) < 0) return sxp_map_error(errno);
  *num_read = read;
  return SXP_SUCCESS;
}

sxpResult sxp_poll(size_t /*maybe NULL*/ *results, pollsxp_t sxps[], size_t sxpcount, int timeout)
{
  ssize_t result;
  if (!sxps) return SXP_ERROR_INVAL;
  if ((result = poll(sxps, sxpcount, timeout)) < 0) return sxp_map_error(errno);
  if (results) *results = result;
  return result > 0 ? SXP_SUCCESS : SXP_TRY_AGAIN;
}

static sxpResult sxp_map_eai_error(int error, int system_errno)
{
  switch (error) {
    case EAI_MEMORY: return SXP_ERROR_MEMORY;
    case EAI_NONAME: return SXP_ERROR_UNKNOWN;
    case EAI_SYSTEM: return sxp_map_error(system_errno);
    case EAI_AGAIN: return SXP_TRY_AGAIN;
    case EAI_BADFLAGS:
    case EAI_FAMILY:
    case EAI_SERVICE:
    case EAI_SOCKTYPE: return SXP_ERROR_INVAL;
    case EAI_FAIL: return SXP_ERROR_PLATFORM;
    default: return SXP_ERROR_UNKNOWN;
  }
  return SXP_ERROR_UNKNOWN;
}

static sxpResult sxp_map_error(int error)
{
  switch (error) {
    case EAFNOSUPPORT:
    case EPROTONOSUPPORT:
    case EPROTOTYPE: return SXP_ERROR_PLATFORM;
    case ENFILE:
    case EMFILE:
    case ENOBUFS: return SXP_ERROR_PLATFORM;
    case ENAMETOOLONG:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case EALREADY:
    case EOPNOTSUPP:
    case EDESTADDRREQ:
    case ENOTSOCK:
    case EISDIR:
    case ELOOP:
    case ENOENT:
    case ENOTDIR:
    case EROFS:
    case EISCONN:
    case EBADF: return SXP_ERROR_INVAL;
    case EACCES: return SXP_ERROR_UNKNOWN;
    case EINVAL: return SXP_ERROR_INVAL;
    case EINTR: return SXP_ERROR_UNKNOWN;
    case ENOMEM: return SXP_ERROR_MEMORY;
    case EWOULDBLOCK: return SXP_TRY_AGAIN;
    case EMSGSIZE: return SXP_TOO_BIG;
    case ECONNABORTED:
    case ECONNREFUSED:
    case ENETUNREACH:
    case ENETDOWN:
    case EIO: return SXP_ERROR_IO;
    case EINPROGRESS: return SXP_ASYNC;
    case ETIMEDOUT: return SXP_TRY_AGAIN;
    case EPIPE: return SXP_ERROR_CLOSED;
    default: return SXP_ERROR_UNKNOWN;
  }
  return SXP_ERROR_UNKNOWN;
}
