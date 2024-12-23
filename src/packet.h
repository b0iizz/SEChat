#ifndef PACKET_H_
#define PACKET_H_

#include "socketxp.h"
#include <stdlib.h>

typedef int parseResult;

enum parseresults {
  PACKET_SUCCESS = 0,
  PACKET_ERROR = 1
};

typedef struct packet_t {
  char *buffer;
  size_t size;
  size_t capacity;
} packet_t;

parseResult packet_realloc(packet_t *pak, size_t capacity);
parseResult packet_free(packet_t *pak);

parseResult packet_recv_i16(packet_t *buf, short int *res);
parseResult packet_recv_i32(packet_t *buf, long int *res);
parseResult packet_recv_str(packet_t *buf, char **res);

parseResult packet_send_i16(packet_t *buf, short int res);
parseResult packet_send_i32(packet_t *buf, long int res);
parseResult packet_send_str(packet_t *buf, const char *res);

#endif /*PACKET_H_*/
