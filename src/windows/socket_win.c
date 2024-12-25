#include "socket_win.h"#include "socket_win.h"
#include "socket_win.h"
#include "..\socketxp.h"
#include <winsock2.h>
#include <ws2tcpip.h>

/*WSAStartup*/
sxpResult sxp_init()
{
	/*todo*/
}

void sxp_cleanup()
{
	int err = WSACleanup();
	if(err = SOCKET_ERROR){
		err = WSAGetLastError();
		return sxp_map_wsa_error(err);
	}
	return SXP_SUCCESS;
}

sxpResult sxp_create(sxp_t *dst, int family, int type, int protocol, int nonblock)
{
	int err;
	SOCKET socket( family, type, protocol);

	if (!dst) return SXP_ERROR_INVAL;
	if(socket == INVALID_SOCKET){
		err = WSAGetLastError();
		return sxp_map_wsa_error(err);
	}
	
	*dst = socket;
	return SXP_SUCCESS;
}

/*closesocket*/
sxpResult sxp_destroy(sxp_t *sock)
{
	int err;
	if (!sock) return SXP_ERROR_INVAL;

	if((err = closesocket(*sock)) != 0){
		err = WSAGetLastError();
		return sxp_map_wsa_error(err);
	}
	return SXP_SUCCESS;
}

/**/
sxpResult sxp_addrinfo_get(addrinfo_t **results, const char *maybe_hostname, const char *serviceport, addrinfo_t *hints);
/**/
sxpResult sxp_addrinfo_free(addrinfo_t *info);

/*server-side API*/
/*bind*/
sxpResult sxp_bind(sxp_t *sock, sockaddr_t *address, size_t addrlen)
{
	int err;
	if (!sock) return SXP_ERROR_INVAL;
	/*todo*/
	if(bind(*sock, address, addrlen) == SOCKET_ERROR){
		err = WSAGetLastError();
		return sxp_map_wsa_error(err);
	}
	return SXP_SUCCESS;
}
/*listen*/
sxpResult sxp_listen(sxp_t *sock){
	int err;
	if (!sock) return SXP_ERROR_INVAL;
	if(listen(*sock, SOMAXCONN) == SOCKET_ERROR){
		err = WSAGetLastError();
		return sxp_map_wsa_error(err);
	}
	return SXP_SUCCESS;
}
/*accept*/
sxpResult sxp_accept(sxp_t *sock, sxp_t *newsock)
{
	int err;
	if (!sock) return SXP_ERROR_INVAL;

	SOCKET new = accept(*sock, NULL, NULL);
	if(new == INVALID_SOCKET ){
		err = WSAGetLastError();
		return sxp_map_wsa_error(err);
	}

	*newsock = new;
	return SXP_SUCCESS;
}

/*client-side API*/
/*connect*/
sxpResult sxp_connect(sxp_t *sock, sockaddr_t *address, size_t addrlen)
{
	int err;
	if (!sock) return SXP_ERROR_INVAL;
	/*todo*/
	if(connect(*sock, address, addrlen) == SOCKET_ERROR){
		err = WSAGetLastError();
		return sxp_map_wsa_error(err);
	}
	return SXP_SUCCESS;
}

/*any-side API*/
/*WSASend*/
sxpResult sxp_send(sxp_t *sock, const char *data, size_t size);
/*WSARecv*/
sxpResult sxp_recv(sxp_t *sock, char *data, size_t size);

/*WSAPoll*/
sxpResult sxp_poll(size_t /*maybe NULL*/ *results, pollsxp_t sxps[], size_t sxpcount, int timeout);

static sxpResult sxp_map_wsa_error(int error)
{
  switch (error) {
    /*case EAFNOSUPPORT:
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
    case EISCONN:*/
    case WSA_INVALID_PARAMETER:
    case WSAEBADF: return SXP_ERROR_INVAL;
    case WSAEACCES: return SXP_ERROR_UNKNOWN;
    case WSAEINVAL: return SXP_ERROR_INVAL;
    case WSAEINTR: return SXP_ERROR_UNKNOWN;
    case WSA_NOT_ENOUGH_MEMORY: return SXP_ERROR_MEMORY;
    case WSAEWOULDBLOCK: return SXP_TRY_AGAIN;
    /*case EMSGSIZE: return SXP_TOO_BIG;
    */case WSAECONNABORTED:
    /*case ECONNREFUSED:*/
    case WSAENETUNREACH:
    case WSAENETDOWN:
    /*case EIO: return SXP_ERROR_IO;
    */case WSAEINPROGRESS: return SXP_ASYNC;
    case WSAETIMEDOUT: return SXP_TRY_AGAIN;
    /*case EPIPE: return SXP_ERROR_CLOSED;*/
    default: return SXP_ERROR_UNKNOWN;
  }
  return SXP_ERROR_UNKNOWN;
}