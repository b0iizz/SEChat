#include "socketxp.h"
#include "net.h"
#include "netio.h"
#include "packet.h"

static struct netio_connection_info {
    connection_t connection;
    sxp_t socket;
    net_buffer_t recv_buffer;
    net_buffer_t send_buffer;
} *netio_connections = NULL;
static connection_t netio_connection_count = 0;

static pollsxp_t *netio_poll_list = NULL;
static size_t netio_active_count = 0;

static int netio_accepts_sockets = 0;

static netResult pull_data(struct netio_connection_info *connection);
static netResult push_data(struct netio_connection_info *connection);

static netResult setup_connection(sxp_t socket);

netResult netio_init()
{
    sxp_init();
    return NET_SUCCESS;
}

netResult netio_exit()
{
    sxp_cleanup();
    return NET_SUCCESS;
}

netResult netio_connect(const char *hostname, const char *port)
{
    addrinfo_t *addresses;
    addrinfo_t hints;
    sxp_t client;
    sxpResult res;
    netResult result = NET_SUCCESS;

    if (netio_connection_count) {
        result = NET_ERROR;
        goto end;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((res = sxp_addrinfo_get(&addresses, hostname, port, &hints)) !=
        SXP_SUCCESS) {
        result = res == SXP_TRY_AGAIN ? NET_TRY_AGAIN : NET_ERROR;
        goto end;
    }

    if ((res = sxp_create(&client, addresses->ai_family, addresses->ai_socktype,
                          addresses->ai_protocol)) != SXP_SUCCESS) {
        result = NET_ERROR;
        goto end_addrinfo;
    }

    if ((res = sxp_connect(&client, addresses->ai_addr,
                           addresses->ai_addrlen)) != SXP_SUCCESS) {
        result = NET_ERROR;
        goto end_socket;
    }

    if ((res = sxp_nbio_set(&client, SXP_NONBLOCKING)) != SXP_SUCCESS) {
        result = NET_ERROR;
        goto end_socket;
    }

    netio_connection_count = 0;

    netio_accepts_sockets = 0;

    if ((result = setup_connection(client)) != NET_SUCCESS)
        goto end_socket;

    goto end_addrinfo;
end_socket:
    (void)sxp_destroy(&client);
end_addrinfo:
    (void)sxp_addrinfo_free(addresses);
end:
    return result;
}

netResult netio_serve(const char *port)
{
    addrinfo_t *addresses;
    addrinfo_t hints;
    sxp_t server;
    sxpResult res;
    netResult result = NET_SUCCESS;

    if (netio_connection_count) {
        result = NET_ERROR;
        goto end;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((res = sxp_addrinfo_get(&addresses, NULL, port, &hints)) !=
        SXP_SUCCESS) {
        result = res == SXP_TRY_AGAIN ? NET_TRY_AGAIN : NET_ERROR;
        goto end;
    }

    if ((res = sxp_create(&server, addresses->ai_family, addresses->ai_socktype,
                          addresses->ai_protocol)) != SXP_SUCCESS) {
        result = NET_ERROR;
        goto end_addrinfo;
    }

    if ((res = sxp_bind(&server, addresses->ai_addr, addresses->ai_addrlen)) !=
        SXP_SUCCESS) {
        result = NET_ERROR;
        goto end_socket;
    }

    if ((res = sxp_listen(&server, NETIO_ACCEPT_BACKLOG)) != SXP_SUCCESS) {
        result = NET_ERROR;
        goto end_socket;
    }

    if ((res = sxp_nbio_set(&server, SXP_NONBLOCKING)) != SXP_SUCCESS) {
        result = NET_ERROR;
        goto end_socket;
    }

    netio_connection_count = 0;

    netio_accepts_sockets = 1;

    if ((result = setup_connection(server)) != NET_SUCCESS)
        goto end_socket;

    goto end_addrinfo;
end_socket:
    (void)sxp_destroy(&server);
end_addrinfo:
    (void)sxp_addrinfo_free(addresses);
end:
    return result;
}

netResult netio_reset()
{
    if (netio_connections) {
        connection_t i;
        for (i = 0; i < netio_connection_count; i++) {
            if (netio_connection_active(i))
                netio_connection_close(i);
        }
        free(netio_connections);
        netio_connections = NULL;
    }
    if (netio_poll_list) {
        free(netio_poll_list);
        netio_poll_list = NULL;
    }
    netio_connection_count = 0;
    netio_active_count = 0;
    netio_accepts_sockets = 0;
    return NET_SUCCESS;
}

netResult netio_tick()
{
    size_t event_count;
    sxpResult result;
    size_t idx;

    if ((result = sxp_poll(&event_count, netio_poll_list, netio_active_count,
                           NETIO_TIMEOUT)) != SXP_SUCCESS) {
        switch (result) {
        case SXP_TRY_AGAIN:
            return NET_SUCCESS;
        default:
            return NET_ERROR;
        }
    }

    idx = 0;

    if (netio_accepts_sockets && netio_active_count) {
        /*server socket is no longer index 0. an error must have occured*/
        if (netio_poll_list[idx].events & SXP_POLLOUT)
            return NET_ERROR;
        /*an error occured on the server socket*/
        if (netio_poll_list[idx].revents & (POLLHUP | POLLERR | POLLNVAL))
            return NET_ERROR;
        /*a new client can be accepted*/
        if (netio_poll_list[idx].revents & POLLIN) {
            sxp_t new_sock;
            if (sxp_accept(&netio_poll_list[idx].fd, &new_sock) ==
                SXP_SUCCESS) {
                if (sxp_nbio_set(&new_sock, SXP_NONBLOCKING) != SXP_SUCCESS) {
                    sxp_destroy(&new_sock);
                } else {
                    setup_connection(new_sock);
                }
            }
        }
        idx++;
    }

    for (; event_count > 0 && idx < netio_active_count;) {
        connection_t conn;
        pollsxp_t *client = &netio_poll_list[idx];
        if (!(client->revents &
              (POLLHUP | POLLERR | POLLNVAL | SXP_POLLIN | SXP_POLLOUT))) {
            idx++;
            continue;
        }
        event_count--;
        for (conn = 0; conn < netio_connection_count; conn++)
            if (netio_connections[conn].socket == client->fd)
                break;

        if (client->revents & (POLLHUP | POLLERR | POLLNVAL)) {
            netio_connection_close(conn);
            continue;
        }
        if ((client->revents & SXP_POLLIN) &&
            (pull_data(&(netio_connections[conn])) == NET_ERROR)) {
            netio_connection_close(conn);
            continue;
        }
        if ((client->revents & SXP_POLLOUT) &&
            (push_data(&(netio_connections[conn])) == NET_ERROR)) {
            netio_connection_close(conn);
            continue;
        }

        idx++;
    }
    return NET_SUCCESS;
}

netResult netio_recv(connection_t *who, net_buffer_t *packet)
{
    connection_t con = netio_accepts_sockets ? 1 : 0;
    for (; con < netio_connection_count; con++) {
        if (netio_connection_active(con)) {
            switch (packet_recv_packet(&(netio_connections[con].recv_buffer),
                                       packet)) {
            case PACKET_NOT_READY:
                continue;
            case PACKET_SUCCESS:
                *who = con;
                return NET_SUCCESS;
            case PACKET_ERROR:
            default:
                return NET_ERROR;
            }
        }
    }
    return NET_TRY_AGAIN;
}

netResult netio_send(connection_t who, const net_buffer_t *packet)
{
    if (!netio_connection_active(who))
        return NET_ERROR;

    if (netio_connections[who].send_buffer.size + packet->size +
            sizeof(unsigned long) >
        NETIO_BUFFER_MAX_SIZE) {
        return NET_ERROR;
    }
    if (packet_send_packet(&(netio_connections[who].send_buffer), packet) !=
        PACKET_SUCCESS)
        return NET_ERROR;
    return NET_SUCCESS;
}

int netio_connection_active(connection_t who)
{
    if (who >= netio_connection_count)
        return 0;
    return netio_connections[who].connection == who;
}

netResult netio_connection_close(connection_t who)
{
    size_t pollidx;

    if (!netio_connection_active(who))
        return NET_ERROR;

    for (pollidx = 0; pollidx < netio_active_count; pollidx++)
        if (netio_poll_list[pollidx].fd == netio_connections[who].socket)
            break;
    if (pollidx == netio_active_count)
        return NET_ERROR;

    if (sxp_destroy(&netio_connections[who].socket) != SXP_SUCCESS)
        return NET_ERROR;
    packet_free(&(netio_connections[who].recv_buffer));
    packet_free(&(netio_connections[who].send_buffer));
    memset(&netio_connections[who], 0, sizeof(netio_connections[0]));
    netio_connections[who].connection = -1;

    memmove(&netio_poll_list[pollidx], &netio_poll_list[netio_active_count - 1],
            sizeof(netio_poll_list[0]));

    if ((netio_active_count - 1)) {
        netio_poll_list =
            realloc(netio_poll_list,
                    sizeof(netio_poll_list[0]) * (netio_active_count - 1));
        if (!netio_poll_list)
            return NET_ERROR;
        netio_active_count -= 1;
    } else {
        free(netio_poll_list);
        netio_poll_list = NULL;
        netio_active_count = 0;
    }

    return NET_SUCCESS;
}

static netResult setup_connection(sxp_t socket)
{
    netio_connections =
        realloc(netio_connections,
                sizeof(netio_connections[0]) * (netio_connection_count + 1));
    if (!netio_connections)
        return NET_ERROR;

    memset(&netio_connections[netio_connection_count], 0,
           sizeof(netio_connections[0]));
    netio_connections[netio_connection_count].connection =
        netio_connection_count;
    netio_connections[netio_connection_count].socket = socket;

    netio_poll_list = realloc(netio_poll_list, sizeof(netio_poll_list[0]) *
                                                   (netio_active_count + 1));
    if (!netio_poll_list)
        return NET_ERROR;

    netio_poll_list[netio_active_count].fd = socket;
    netio_poll_list[netio_active_count].events = SXP_POLLIN;
    netio_poll_list[netio_active_count].revents = 0;

    /*socket is not the server socket*/
    if (!netio_accepts_sockets || netio_active_count) {
        netio_poll_list[netio_active_count].events |= SXP_POLLOUT;
    } else {
        netio_poll_list[netio_active_count].events |= POLLIN;
    }
    netio_active_count += 1;
    netio_connection_count += 1;
    return NET_SUCCESS;
}

static netResult pull_data(struct netio_connection_info *connection)
{
    size_t new_capacity;
    size_t old_capacity;
    size_t num_read;
    sxpResult result;
    char buf[NETIO_READ_MAX];

    while ((result = sxp_recv(&(connection->socket), buf, &num_read,
                              NETIO_READ_MAX)) == SXP_SUCCESS) {
        if (num_read == 0)
            return NET_ERROR;
        old_capacity = connection->recv_buffer.capacity;
        new_capacity = connection->recv_buffer.capacity + num_read;
        if (new_capacity > NETIO_BUFFER_MAX_SIZE) {
            old_capacity -= connection->recv_buffer.size;
            new_capacity -= connection->recv_buffer.size;
            memmove(connection->recv_buffer.buffer,
                    connection->recv_buffer.buffer +
                        connection->recv_buffer.size,
                    old_capacity);
            connection->recv_buffer.size = 0;
            if (new_capacity > NETIO_BUFFER_MAX_SIZE)
                return NET_ERROR;
        }
        if (packet_realloc(&(connection->recv_buffer), new_capacity) !=
            PACKET_SUCCESS)
            return NET_ERROR;
        memcpy(connection->recv_buffer.buffer + old_capacity, buf, num_read);
    }
    if (result == SXP_TRY_AGAIN)
        return NET_SUCCESS;
    return NET_ERROR;
}

static netResult push_data(struct netio_connection_info *connection)
{
    size_t msg_size = connection->send_buffer.size;
    sxpResult result;

    while ((result = sxp_send(&(connection->socket),
                              connection->send_buffer.buffer, msg_size)) ==
           SXP_TOO_BIG) {
        msg_size /= 2;
    }
    if (result == SXP_TRY_AGAIN)
        return NET_TRY_AGAIN;
    if (result != SXP_SUCCESS)
        return NET_ERROR;
    connection->send_buffer.size -= msg_size;
    memmove(connection->send_buffer.buffer,
            connection->send_buffer.buffer + msg_size,
            connection->send_buffer.size);
    return NET_SUCCESS;
}
