#include "packet.h"
#include <limits.h>
#include <string.h>

static parseResult packet_recv_buf(net_buffer_t *pak, char **buf, size_t *size);
static parseResult packet_send_buf(net_buffer_t *pak, const char *buf, size_t size);

parseResult packet_realloc(net_buffer_t *pak, size_t capacity)
{
  if (pak->size > capacity) return PACKET_ERROR;
  pak->buffer = realloc(pak->buffer, capacity);
  if (!(pak->buffer)) return PACKET_ERROR;
  pak->capacity = capacity;
  return PACKET_SUCCESS;
}

parseResult packet_free(net_buffer_t *pak)
{
  free(pak->buffer);
  memset(pak, 0, sizeof(*pak));
  return PACKET_SUCCESS;
}

parseResult packet_recv_i32(net_buffer_t *pak, long int *res)
{
  unsigned long ures;
  parseResult result = packet_recv_u32(pak, &ures);
  if (result == PACKET_SUCCESS) *res = ures;
  return result;
}

parseResult packet_recv_u32(net_buffer_t *pak, unsigned long *res)
{
  if (pak->size + sizeof(unsigned long) > pak->capacity) return PACKET_NOT_READY;
  *res = 0;
  *res |= ((unsigned char) pak->buffer[pak->size + 0]) << 24;
  *res |= ((unsigned char) pak->buffer[pak->size + 1]) << 16;
  *res |= ((unsigned char) pak->buffer[pak->size + 2]) << 8;
  *res |= ((unsigned char) pak->buffer[pak->size + 3]) << 0;
  pak->size += sizeof(unsigned long);
  return PACKET_SUCCESS;
}

parseResult packet_send_i32(net_buffer_t *pak, long int n) { return packet_send_u32(pak, n); }


parseResult packet_send_u32(net_buffer_t *pak, unsigned long n)
{
  parseResult parsed;
  if ((pak->size + sizeof(unsigned long) > pak->capacity)
      && (parsed = packet_realloc(pak, pak->capacity + sizeof(unsigned long))) != PACKET_SUCCESS)
    return parsed;
  pak->buffer[pak->size + 0] = (unsigned char) ((n >> 24) & 0xFF);
  pak->buffer[pak->size + 1] = (unsigned char) ((n >> 16) & 0xFF);
  pak->buffer[pak->size + 2] = (unsigned char) ((n >> 8) & 0xFF);
  pak->buffer[pak->size + 3] = (unsigned char) ((n >> 0) & 0xFF);
  pak->size += sizeof(unsigned long);
  return PACKET_SUCCESS;
}

parseResult packet_recv_str(net_buffer_t *pak, char **res)
{
  unsigned long len;
  return packet_recv_buf(pak, res, &len);
}

parseResult packet_send_str(net_buffer_t *pak, const char *str)
{
  return packet_send_buf(pak, str, strlen(str) + 1);
}

parseResult packet_recv_packet(net_buffer_t *pak, net_buffer_t *buf)
{
  parseResult result;
  unsigned long capacity;
  if ((result = packet_recv_buf(pak, &(buf->buffer), &capacity)) != PACKET_SUCCESS) return result;
  buf->size = 0;
  buf->capacity = capacity;
  return result;
}

parseResult packet_send_packet(net_buffer_t *pak, const net_buffer_t *buf)
{
  return packet_send_buf(pak, buf->buffer, buf->size);
}

static parseResult packet_recv_buf(net_buffer_t *pak, char **buf, size_t *size)
{
  long int length;
  parseResult parsed;
  if ((parsed = packet_recv_i32(pak, &length)) != PACKET_SUCCESS) return parsed;
  if (length < 0 || (pak->size + length > pak->capacity)) {
    pak->size -= sizeof(unsigned long);
    return PACKET_NOT_READY;
  }
  *buf = malloc(length);
  if (!(*buf)) return PACKET_ERROR;
  memcpy(*buf, pak->buffer + pak->size, length);
  *size = length;
  pak->size += length;
  return PACKET_SUCCESS;
}


static parseResult packet_send_buf(net_buffer_t *pak, const char *buf, size_t size)
{
  parseResult parsed;
  if (size >= (size_t) LONG_MAX) return PACKET_ERROR;
  if ((parsed = packet_send_i32(pak, size)) != PACKET_SUCCESS) return parsed;
  if ((pak->size + size > pak->capacity)
      && (parsed = packet_realloc(pak, pak->capacity + size)) != PACKET_SUCCESS)
    return parsed;
  memcpy(pak->buffer + pak->size, buf, size);
  pak->size += size;
  return PACKET_SUCCESS;
}
