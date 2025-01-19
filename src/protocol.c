#include "protocol.h"

static parseResult protocol_packet_recv_handshake_c(net_buffer_t *protocol_packet, struct protocol_packet_handshake_c *data);
static parseResult protocol_packet_recv_info_c(net_buffer_t *protocol_packet, struct protocol_packet_info_c *data);
static parseResult protocol_packet_recv_handshake_s(net_buffer_t *protocol_packet, struct protocol_packet_handshake_s *data);
static parseResult protocol_packet_recv_person(net_buffer_t *protocol_packet, struct protocol_packet_person *data);
static parseResult protocol_packet_recv_message(net_buffer_t *protocol_packet, struct protocol_packet_message *data);

static parseResult protocol_packet_send_handshake_c(net_buffer_t *protocol_packet, const struct protocol_packet_handshake_c *data);
static parseResult protocol_packet_send_info_c(net_buffer_t *protocol_packet, const struct protocol_packet_info_c *data);
static parseResult protocol_packet_send_handshake_s(net_buffer_t *protocol_packet, const struct protocol_packet_handshake_s *data);
static parseResult protocol_packet_send_person(net_buffer_t *protocol_packet, const struct protocol_packet_person *data);
static parseResult protocol_packet_send_message(net_buffer_t *protocol_packet, const struct protocol_packet_message *data);

parseResult packet_deserialize(net_buffer_t *protocol_packet, struct protocol_packet *res)
{
  long int type;
  packet_recv_i32(protocol_packet, &type);
  res->type = type;
  switch (res->type) {
    case NET_PROTO_HANDSHAKE_C: protocol_packet_recv_handshake_c(protocol_packet, &(res->as.handshake_c)); break;
    case NET_PROTO_HANDSHAKE_S: protocol_packet_recv_handshake_s(protocol_packet, &(res->as.handshake_s)); break;
    case NET_PROTO_PERSON: protocol_packet_recv_person(protocol_packet, &(res->as.person)); break;
    case NET_PROTO_MESSAGE: protocol_packet_recv_message(protocol_packet, &(res->as.message)); break;
    case NET_PROTO_INFO_C: protocol_packet_recv_info_c(protocol_packet, &(res->as.info_c)); break;
    default: return PACKET_ERROR;
  }
  return PACKET_SUCCESS;
}

parseResult packet_serialize(net_buffer_t *protocol_packet, const struct protocol_packet *data)
{
  packet_send_i32(protocol_packet, data->type);
  switch (data->type) {
    case NET_PROTO_HANDSHAKE_C: protocol_packet_send_handshake_c(protocol_packet, &(data->as.handshake_c)); break;
    case NET_PROTO_HANDSHAKE_S: protocol_packet_send_handshake_s(protocol_packet, &(data->as.handshake_s)); break;
    case NET_PROTO_PERSON: protocol_packet_send_person(protocol_packet, &(data->as.person)); break;
    case NET_PROTO_MESSAGE: protocol_packet_send_message(protocol_packet, &(data->as.message)); break;
    case NET_PROTO_INFO_C: protocol_packet_send_info_c(protocol_packet, &(data->as.info_c)); break;
    default: return PACKET_ERROR;
  }
  return PACKET_SUCCESS;
}

static parseResult protocol_packet_recv_handshake_c(net_buffer_t *protocol_packet, struct protocol_packet_handshake_c *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_recv_u32(protocol_packet, &(data->proto_ver));
  if (res == PACKET_SUCCESS) res = packet_recv_u32(protocol_packet, &(data->proto_flags));
  return res;
}

static parseResult protocol_packet_recv_info_c(net_buffer_t *protocol_packet, struct protocol_packet_info_c *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_recv_u32(protocol_packet, &(data->info_type));
  return res;
}

static parseResult protocol_packet_recv_handshake_s(net_buffer_t *protocol_packet, struct protocol_packet_handshake_s *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_recv_u32(protocol_packet, &(data->proto_ver));
  if (res == PACKET_SUCCESS) res = packet_recv_u32(protocol_packet, &(data->proto_flags));
  if (res == PACKET_SUCCESS) res = packet_recv_u32(protocol_packet, &(data->self_id));
  return res;
}

static parseResult protocol_packet_recv_person(net_buffer_t *protocol_packet, struct protocol_packet_person *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_recv_u32(protocol_packet, &(data->person_id));
  if (res == PACKET_SUCCESS) res = packet_recv_str(protocol_packet, &(data->name));
  return res;
}

static parseResult protocol_packet_recv_message(net_buffer_t *protocol_packet, struct protocol_packet_message *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_recv_u32(protocol_packet, &(data->person_id));
  if (res == PACKET_SUCCESS) res = packet_recv_u32(protocol_packet, &(data->encryption));
  if (res == PACKET_SUCCESS) res = packet_recv_i32(protocol_packet, &(data->index));
  if (res == PACKET_SUCCESS) res = packet_recv_str(protocol_packet, &(data->message));
  return res;
}

static parseResult protocol_packet_send_handshake_c(net_buffer_t *protocol_packet, const struct protocol_packet_handshake_c *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_send_u32(protocol_packet, data->proto_ver);
  if (res == PACKET_SUCCESS) res = packet_send_u32(protocol_packet, data->proto_flags);
  return res;
}

static parseResult protocol_packet_send_info_c(net_buffer_t *protocol_packet, const struct protocol_packet_info_c *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_send_u32(protocol_packet, data->info_type);
  return res;
}

static parseResult protocol_packet_send_handshake_s(net_buffer_t *protocol_packet, const struct protocol_packet_handshake_s *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_send_u32(protocol_packet, data->proto_ver);
  if (res == PACKET_SUCCESS) res = packet_send_u32(protocol_packet, data->proto_flags);
  if (res == PACKET_SUCCESS) res = packet_send_u32(protocol_packet, data->self_id);
  return res;
}

static parseResult protocol_packet_send_person(net_buffer_t *protocol_packet, const struct protocol_packet_person *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_send_u32(protocol_packet, data->person_id);
  if (res == PACKET_SUCCESS) res = packet_send_str(protocol_packet, data->name);
  return res;
}

static parseResult protocol_packet_send_message(net_buffer_t *protocol_packet, const struct protocol_packet_message *data)
{
  parseResult res = PACKET_SUCCESS;
  if (res == PACKET_SUCCESS) res = packet_send_u32(protocol_packet, data->person_id);
  if (res == PACKET_SUCCESS) res = packet_send_u32(protocol_packet, data->encryption);
  if (res == PACKET_SUCCESS) res = packet_send_i32(protocol_packet, data->index);
  if (res == PACKET_SUCCESS) res = packet_send_str(protocol_packet, data->message);
  return res;
}
