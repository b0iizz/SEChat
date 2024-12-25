#include "socket_win.h"#include "socket_win.h"
#include "socket_win.h"
#include "..\socketxp.h"
#include <winsock2.h>
#include <ws2tcpip.h>

sxpResult sxp_init();

void sxp_cleanup()
{
	WSACleanup();
}

sxpResult sxp_create(sxp_t *dst, int family, int type, int protocol, int nonblock)
{
	SOCKET WSAAPI socket(
  [in] int af,
  [in] int type,
  [in] int protocol
);

	return *socket;
}
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