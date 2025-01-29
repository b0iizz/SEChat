#ifndef PACKET_H_
#define PACKET_H_

#include <stdlib.h>

typedef int parseResult;

enum parseresults {
    PACKET_SUCCESS = 0,
    PACKET_ERROR = -1,
    PACKET_NOT_READY = 1
};

typedef struct net_buffer_t {
    char *buffer;
    size_t size;
    size_t capacity;
} net_buffer_t;

parseResult packet_realloc(net_buffer_t *pak, size_t capacity);
parseResult packet_free(net_buffer_t *pak);

parseResult packet_recv_u32(net_buffer_t *pak, unsigned long *res);
parseResult packet_recv_i32(net_buffer_t *pak, long int *res);
parseResult packet_recv_str(net_buffer_t *pak, char **res);

parseResult packet_send_u32(net_buffer_t *pak, unsigned long n);
parseResult packet_send_i32(net_buffer_t *pak, long int n);
parseResult packet_send_str(net_buffer_t *pak, const char *str);

parseResult packet_send_packet(net_buffer_t *pak, const net_buffer_t *buf);
parseResult packet_recv_packet(net_buffer_t *pak, net_buffer_t *buf);

#endif /*PACKET_H_*/
