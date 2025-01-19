#include "socketxp.h"


static sxpResult sxp_map_error(int error);
static sxpResult sxp_map_eai_error(int error);

sxpResult sxp_init()
{
  WORD versionReq;
  WSADATA wsaData;

  versionReq = MAKEWORD(2, 2);

  if (WSAStartup(versionReq, &wsaData)) return SXP_ERROR_PLATFORM;
  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
    sxp_cleanup();
    return SXP_ERROR_PLATFORM;
  }
  return SXP_SUCCESS;
}

sxpResult sxp_cleanup()
{
  if (WSACleanup()) return sxp_map_error(WSAGetLastError());
  return SXP_SUCCESS;
}

sxpResult sxp_create(sxp_t *dst, int family, int type, int protocol)
{
  if (!dst) return SXP_ERROR_INVAL;
  *dst = socket(family, type, protocol);
  if (*dst == INVALID_SOCKET) return sxp_map_error(WSAGetLastError());
  return SXP_SUCCESS;
}

sxpResult sxp_destroy(sxp_t *sock)
{
  if (!sock) return SXP_ERROR_INVAL;
  if (closesocket(*sock)) return sxp_map_error(WSAGetLastError());
  return SXP_SUCCESS;
}

sxpResult sxp_nbio_set(sxp_t *sock, int nonblockingio) {
  unsigned long int mode = (nonblockingio == SXP_NONBLOCKING) ? 1 : 0;
  if (!sock) return SXP_ERROR_INVAL;
  if (ioctlsocket(*sock, FIONBIO, &mode)) return sxp_map_error(WSAGetLastError());
  return SXP_SUCCESS;
}

sxpResult sxp_addrinfo_get(addrinfo_t **results, const char *maybe_hostname,
                           const char *serviceport, addrinfo_t *hints)
{
  int result = getaddrinfo(maybe_hostname, serviceport, hints, results);
  if (result != 0) { return sxp_map_eai_error(result); }
  return SXP_SUCCESS;
}

sxpResult sxp_addrinfo_free(addrinfo_t *info)
{
  freeaddrinfo(info);
  return SXP_SUCCESS;
}


/*server-side API*/
sxpResult sxp_bind(sxp_t *sock, const sockaddr_t *address, size_t addrlen)
{
  int yes = 1;
  if (!sock) return SXP_ERROR_INVAL;
  if (setsockopt(*sock, SOL_SOCKET, SO_REAUSEADDR, &yes, sizeof(yes))) return sxp_map_error(WSAGetLastError());
  if (bind(*sock, address, addrlen)) return sxp_map_error(WSAGetLastError());
  return SXP_SUCCESS;
}

sxpResult sxp_listen(sxp_t *sock, size_t backlog)
{
  if (!sock) return SXP_ERROR_INVAL;
  if (listen(*sock, backlog)) return sxp_map_error(WSAGetLastError());
  return SXP_SUCCESS;
}

sxpResult sxp_accept(sxp_t *sock, sxp_t *newsock)
{
  if (!sock || !newsock) return SXP_ERROR_INVAL;
  if ((*newsock = accept(*sock, NULL, NULL)) == INVALID_SOCKET)
    return sxp_map_error(WSAGetLastError());
  return SXP_SUCCESS;
}

/*client-side API*/
sxpResult sxp_connect(sxp_t *sock, sockaddr_t *address, size_t addrlen) {
  if (!sock) return SXP_ERROR_INVAL;
  if (connect(*sock, address, addrlen)) return sxp_map_error(WSAGetLastError());
  return SXP_SUCCESS;

}

/*any-side API*/
sxpResult sxp_send(sxp_t *sock, const char *data, size_t size) {
  if (!sock) return SXP_ERROR_INVAL;
  if (send(*sock, data, size, 0)) return sxp_map_error(WSAGetLastError());
  return SXP_SUCCESS;
}

sxpResult sxp_recv(sxp_t *sock, char *data, size_t *num_read, size_t size) {
  int read;
  if (!sock || !num_read) return SXP_ERROR_INVAL;
  if ((read = recv(*sock, data, size, 0)) == SOCKET_ERROR) return sxp_map_error(WSAGetLastError());
  *num_read = read;
  return SXP_SUCCESS;
}

sxpResult sxp_poll(size_t /*maybe NULL*/ *results, pollsxp_t sxps[], size_t sxpcount, int timeout) {
  int res = WSAPoll(sxps, sxpcount, timeout);
  if (res == SOCKET_ERROR) return sxp_map_error(WSAGetLastError());
  if (results) *results = res;
  return res > 0 ? SXP_SUCCESS : SXP_TRY_AGAIN;
}

static sxpResult sxp_map_eai_error(int error)
{
  switch (error) {
    case EAI_MEMORY: return SXP_ERROR_MEMORY;
    case EAI_NONAME: return SXP_ERROR_UNKNOWN;
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
    case WSAENOPROTOOPT:
    case WSAEISCONN:
    case WSAEADDRINUSE:
    case WSAEADDRNOTAVAIL:
    case WSAEFAULT:
    case WSAENOTCONN:
    case WSAENOTSOCK:
    case WSAEALREADY:
    case WSAEACCES:
    case WSAEINVAL: return SXP_ERROR_INVAL;
    case WSAEINTR: return SXP_ERROR_UNKNOWN;
    case WSAEWOULDBLOCK: return SXP_TRY_AGAIN;
    case WSAEINVALIDPROCTABLE:
    case WSAEINVALIDPROVIDER:
    case WSAEMFILE:
    case WSAEAFNOSUPPORT:
    case WSAEPROTONOSUPPORT:
    case WSAEPROTOTYPE:
    case WSAEPROVIDERFAILEDINIT:
    case WSAESOCKTNOSUPPORT:
    case WSASYSNOTREADY:
    case WSAVERNOTSUPPORTED:
    case WSAEPROCLIM:
    case WSAEOPNOTSUPP:
    case WSANOTINITIALISED: return SXP_ERROR_PLATFORM;
    case WSAEMSGSIZE: return SXP_TOO_BIG;
    case WSAECONNRESET:
    case WSAECONNREFUSED:
    case WSAECONNABORTED:
    case WSAENETUNREACH:
    case WSAEHOSTUNREACH:
    case WSAENETRESET:
    case WSAENETDOWN: return SXP_ERROR_IO;
    case WSAESHUTDOWN:
    case WSAETIMEDOUT: return SXP_ERROR_CLOSED;
    case WSAEINPROGRESS: return SXP_ASYNC;
    case WSAENOBUFS: return SXP_ERROR_MEMORY;
    default: return SXP_ERROR_UNKNOWN;
  }
  return SXP_ERROR_UNKNOWN;
}
