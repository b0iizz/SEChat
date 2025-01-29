#ifndef NETIO_H_
#define NETIO_H_

#include "net.h"
#include "packet.h"

#define NETIO_READ_MAX (16 * 1024)
#define NETIO_BUFFER_MAX_SIZE (4 * 1024 * 1024)
#define NETIO_ACCEPT_BACKLOG 5
#define NETIO_TIMEOUT 10

typedef unsigned int connection_t;

netResult netio_init();
netResult netio_exit();

netResult netio_connect(const char *hostname, const char *port);
netResult netio_serve(const char *port);
netResult netio_reset();

int netio_connection_active(connection_t who);
netResult netio_connection_close(connection_t who);

netResult netio_tick();

netResult netio_recv(connection_t *who, net_buffer_t *packet);
netResult netio_send(connection_t who, const net_buffer_t *packet);

#endif /* NETIO_H_ */
