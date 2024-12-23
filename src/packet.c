#include "packet.h"
#include <limits.h>
#include <string.h>

parseResult packet_realloc(packet_t *pak, size_t capacity)
{
  if (pak->size > capacity) return PACKET_ERROR;
  pak->buffer = realloc(pak->buffer, capacity);
  if (!(pak->buffer)) return PACKET_ERROR;
  pak->capacity = capacity;
  return PACKET_SUCCESS;
}

parseResult packet_free(packet_t *pak)
{
  free(pak->buffer);
  memset(pak, 0, sizeof(*pak));
  return PACKET_SUCCESS;
}

parseResult packet_recv_i16(packet_t *pak, short int *res)
{
  if (pak->size + sizeof(short int) > pak->capacity) return PACKET_ERROR;
  *res = 0;
  *res |= pak->buffer[pak->size + 0] << 8;
  *res |= pak->buffer[pak->size + 1] << 0;
  *res = ntohs(*res);
  pak->size += sizeof(short int);
  return PACKET_SUCCESS;
}

parseResult packet_send_i16(packet_t *pak, short int n)
{
  parseResult parsed;
  short int net = htons(n);
  if ((pak->size + sizeof(short int) > pak->capacity)
      && (parsed = packet_realloc(pak, pak->capacity + sizeof(short int))) != PACKET_SUCCESS)
    return parsed;

  pak->buffer[pak->size + 0] = (net >> 8) & 0xFF;
  pak->buffer[pak->size + 1] = (net >> 0) & 0xFF;
  pak->size += sizeof(short int);
  return PACKET_SUCCESS;
}

parseResult packet_recv_i32(packet_t *pak, long int *res)
{
  if (pak->size + sizeof(long int) > pak->capacity) return PACKET_ERROR;
  *res = 0;
  *res |= pak->buffer[pak->size + 0] << 24;
  *res |= pak->buffer[pak->size + 1] << 16;
  *res |= pak->buffer[pak->size + 2] << 8;
  *res |= pak->buffer[pak->size + 3] << 0;
  *res = ntohl(*res);
  pak->size += sizeof(long int);
  return PACKET_SUCCESS;
}

parseResult packet_send_i32(packet_t *pak, long int n)
{
  parseResult parsed;
  long int net = htonl(n);
  if ((pak->size + sizeof(long int) > pak->capacity)
      && (parsed = packet_realloc(pak, pak->capacity + sizeof(long int))) != PACKET_SUCCESS)
    return parsed;
  pak->buffer[pak->size + 0] = (net >> 24) & 0xFF;
  pak->buffer[pak->size + 1] = (net >> 16) & 0xFF;
  pak->buffer[pak->size + 2] = (net >> 8) & 0xFF;
  pak->buffer[pak->size + 3] = (net >> 0) & 0xFF;
  pak->size += sizeof(long int);
  return PACKET_SUCCESS;
}

parseResult packet_recv_str(packet_t *pak, char **res)
{
  short int length;
  parseResult parsed;
  if ((parsed = packet_recv_i16(pak, &length)) != PACKET_SUCCESS) return parsed;
  if (length < 0 || (pak->size + length > pak->capacity)) return PACKET_ERROR;
  *res = malloc(length);
  if (!(*res)) return PACKET_ERROR;
  memcpy(*res, pak->buffer + pak->size, length);
  pak->size += length;
  return PACKET_SUCCESS;
}

parseResult packet_send_str(packet_t *pak, const char *str)
{
  short int length;
  parseResult parsed;
  if (strlen(str) >= (size_t) SHRT_MAX) return PACKET_ERROR;
  length = strlen(str) + 1;
  if ((parsed = packet_send_i16(pak, length)) != PACKET_SUCCESS) return parsed;
  if ((pak->size + length > pak->capacity)
      && (parsed = packet_realloc(pak, pak->capacity + length)) != PACKET_SUCCESS)
    return parsed;
  memcpy(pak->buffer + pak->size, str, length);
  pak->size += length;
  return PACKET_SUCCESS;
}
