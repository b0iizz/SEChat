#include "net.h"
#include "netio.h"
#include "protocol.h"
#include "util.h"

static int is_server = -1;
static long int self_person_id = -1;

static struct net_message *messages = NULL;
static size_t messages_count = 0;
static size_t message_last_seen = -1;

static char **person_name = NULL;
static size_t person_count = 0;

static char **person_encrypt_plain[ENCRYPT_MAX_VAL] = {0};
static void **person_encrypt_key[ENCRYPT_MAX_VAL] = {0};

static int person_exists(int who);
static netResult person_make(int who);
static netResult person_free(int who);

static netResult connection_close(connection_t who);

static netResult messages_set(long int index, long int person_id, long int encryption,
                              const char *message);

static netResult broadcast(const net_buffer_t *broadcast);

static netResult handle_packet_handshake_c(connection_t sender, struct protocol_packet *packet);
static netResult handle_packet_handshake_s(connection_t sender, struct protocol_packet *packet);
static netResult handle_packet_info_c(connection_t sender, struct protocol_packet *packet);
static netResult handle_packet_person(connection_t sender, struct protocol_packet *packet);
static netResult handle_packet_message(connection_t sender, struct protocol_packet *packet);

netResult net_init() { return netio_init(); }
netResult net_exit()
{
  net_reset();
  return netio_exit();
}

netResult net_connect(const char *hostname, const char *port)
{
  netResult result;
  net_buffer_t packet = {0};
  struct protocol_packet handshake = {0};

  if (is_server >= 0) return NET_ERROR;
  is_server = 0;
  self_person_id = -1;

  if ((result = netio_connect(hostname, port)) != NET_SUCCESS) return result;

  handshake.type = NET_PROTO_HANDSHAKE_C;
  handshake.as.handshake_c.proto_ver = 0;
  handshake.as.handshake_c.proto_flags = 0;
  if (packet_serialize(&packet, &handshake) != PACKET_SUCCESS) return NET_ERROR;

  if ((result = netio_send(0, &packet)) != NET_SUCCESS) return result;
  if (packet_free(&packet) != PACKET_SUCCESS) return NET_ERROR;

  return result;
}

netResult net_serve(const char *port)
{
  netResult result;

  if (is_server >= 0) return NET_ERROR;
  is_server = 1;
  self_person_id = 0;

  result = netio_serve(port);
  if (result == NET_SUCCESS) result = person_make(self_person_id);

  return result;
}

netResult net_reset()
{
  size_t i, j;

  if (is_server < 0) return NET_SUCCESS;

  for (i = 0; i < messages_count; i++) free(messages[i].message);
  free(messages);
  messages = NULL;
  messages_count = 0;
  message_last_seen = -1;

  for (i = 0; i < ENCRYPT_MAX_VAL; i++) {
    for (j = 0; j < person_count; j++)
      if (person_name[j]) {
        free(person_encrypt_plain[i][j]);
        encryptors[i].key_free(person_encrypt_key[i][j]);
      }
    free(person_encrypt_plain[i]);
    free(person_encrypt_key[i]);
  }
  memset(person_encrypt_plain, 0, sizeof(*person_encrypt_plain) * ENCRYPT_MAX_VAL);
  memset(person_encrypt_key, 0, sizeof(*person_encrypt_key) * ENCRYPT_MAX_VAL);

  for (i = 0; i < person_count; i++) free(person_name[i]);
  free(person_name);
  person_name = NULL;
  person_count = 0;

  is_server = -1;
  self_person_id = -1;

  return netio_reset();
}

netResult net_tick()
{
  netResult result;

  connection_t sender = 0;
  net_buffer_t incoming = {0};
  struct protocol_packet request = {0};

  if (is_server < 0) return NET_SUCCESS;

  if ((result = netio_tick()) != NET_SUCCESS) return result;
  while ((result = netio_recv(&sender, &incoming)) == NET_SUCCESS) {
    while (incoming.size < incoming.capacity
           && packet_deserialize(&incoming, &request) == PACKET_SUCCESS) {
      switch (request.type) {
        case NET_PROTO_HANDSHAKE_C: result = handle_packet_handshake_c(sender, &request); break;
        case NET_PROTO_HANDSHAKE_S: result = handle_packet_handshake_s(sender, &request); break;
        case NET_PROTO_INFO_C: result = handle_packet_info_c(sender, &request); break;
        case NET_PROTO_PERSON:
          result = handle_packet_person(sender, &request);
          free(request.as.person.name);
          break;
        case NET_PROTO_MESSAGE:
          result = handle_packet_message(sender, &request);
          free(request.as.message.message);
          break;
        default: result = NET_ERROR; break;
      }
      if (result != NET_SUCCESS) {
        connection_close(sender);
        break;
      }
    }
    packet_free(&incoming);
  }
  if (result == NET_TRY_AGAIN) return NET_SUCCESS;
  return result;
}

netResult net_name_set(int person, const char *name)
{
  netResult result;
  net_buffer_t outgoing = {0};
  struct protocol_packet update = {0};

  if (is_server < 0) return NET_ERROR;

  person = person == NET_MYSELF ? self_person_id : person;

  if (is_server && !person_exists(person)) return NET_ERROR;
  if (!is_server && person != self_person_id) return NET_ERROR;
  if (!is_server && person < 0) return NET_ERROR;

  update.type = NET_PROTO_PERSON;
  update.as.person.person_id = person;
  update.as.person.name = NULL;
  result = util_strcpy(&(update.as.person.name), name, NET_SUCCESS, NET_ERROR);

  if (result == NET_SUCCESS)
    result = packet_serialize(&outgoing, &update) == PACKET_SUCCESS ? NET_SUCCESS : NET_ERROR;

  if (result == NET_SUCCESS) {
    if (is_server) {
      result = broadcast(&outgoing);
      if (result == NET_SUCCESS) result = handle_packet_person(person, &update);
    } else {
      result = netio_send(0, &outgoing);
    }
  }

  free(update.as.person.name);
  packet_free(&outgoing);

  return result;
}

netResult net_name_get(int person, char **name)
{
  netResult result;
  net_buffer_t outgoing = {0};
  struct protocol_packet query = {0};

  if (is_server < 0) return NET_ERROR;

  person = person == NET_MYSELF ? self_person_id : person;

  if (!person_exists(person)) {
    if (is_server) return NET_ERROR;
    if (person >= 0 && (size_t) person < person_count) return NET_ERROR;

    query.type = NET_PROTO_INFO_C;
    query.as.info_c.info_type = NET_PINFO_AUDIENCE;

    result = packet_serialize(&outgoing, &query) == PACKET_SUCCESS ? NET_SUCCESS : NET_ERROR;
    if (result == NET_SUCCESS) result = netio_send(0, &outgoing);

    return result != NET_ERROR ? NET_TRY_AGAIN : NET_ERROR;
  }

  result = util_strcpy(name, person_name[person], NET_SUCCESS, NET_ERROR);
  return result;
}

netResult net_key_set(int person, int method, const char *key)
{
  netResult result;

  if (is_server < 0) return NET_ERROR;
  if (method < 0 || method >= ENCRYPT_MAX_VAL) return NET_ERROR;

  person = person == NET_MYSELF ? self_person_id : person;

  if (!person_exists(person)) return NET_ERROR;

  if (person_encrypt_plain[method][person]) {
    free(person_encrypt_plain[method][person]);
    encryptors[method].key_free(person_encrypt_key[method][person]);
  }

  result = util_strcpy(&person_encrypt_plain[method][person], key, NET_SUCCESS, NET_ERROR);
  if (result == NET_SUCCESS) {
    person_encrypt_key[method][person] = encryptors[method].key_parse(key);
    if (!person_encrypt_key[method][person]) {
      free(person_encrypt_plain[method][person]);
      person_encrypt_plain[method][person] = NULL;
    }
  }
  return result;
}

netResult net_key_get(int person, int method, char **dst)
{
  netResult result;

  if (is_server < 0) return NET_ERROR;
  if (method < 0 || method >= ENCRYPT_MAX_VAL) return NET_ERROR;

  person = person == NET_MYSELF ? self_person_id : person;

  if (!person_exists(person)) return NET_ERROR;

  if (!person_encrypt_plain[method][person]) {
    *dst = NULL;
    return NET_SUCCESS;
  }

  result = util_strcpy(dst, person_encrypt_plain[method][person], NET_SUCCESS, NET_ERROR);

  return result;
}

netResult net_message_send(int encryption, const char *message)
{
  netResult result;
  net_buffer_t outgoing = {0};
  struct protocol_packet packet = {0};

  if (is_server < 0) return NET_ERROR;
  if (!person_exists(self_person_id)) return NET_TRY_AGAIN;

  packet.type = NET_PROTO_MESSAGE;
  packet.as.message.person_id = self_person_id;
  packet.as.message.index = -1;
  packet.as.message.encryption =
      person_encrypt_plain[encryption][self_person_id] ? encryption : ENCRYPT_NONE;
  packet.as.message.message = NULL;
  result = util_strcpy(&packet.as.message.message, message, NET_SUCCESS, NET_ERROR);

  if (person_encrypt_plain[encryption][self_person_id]) {
    encryptors[encryption].encode(&packet.as.message.message,
                                  person_encrypt_key[encryption][self_person_id]);
  }

  if (result == NET_SUCCESS)
    result = packet_serialize(&outgoing, &packet) == PACKET_SUCCESS ? NET_SUCCESS : NET_ERROR;
  if (result == NET_SUCCESS) {
    if (is_server) {
      result = broadcast(&outgoing);
      if (result == NET_SUCCESS) result = handle_packet_message(self_person_id, &packet);
    } else {
      result = netio_send(0, &outgoing);
    }
  }

  free(packet.as.message.message);
  packet_free(&outgoing);
  return result;
}

netResult net_message_recv(struct net_message *buffer, size_t *count, size_t limit, int flags)
{
  netResult result = NET_SUCCESS;
  ssize_t read_start;
  size_t max_count;
  size_t idx;

  if (is_server < 0) return NET_TRY_AGAIN;

  if (flags & NET_FHISTORY) {
    read_start = message_last_seen - limit;
    read_start = read_start > 0 ? read_start : 0;
    max_count = message_last_seen - read_start + 1;
  } else {
    read_start = message_last_seen + 1;
    max_count = messages_count - read_start;
    max_count = max_count > limit ? limit : max_count;
    message_last_seen += max_count;
  }

  if (max_count == 0) return NET_TRY_AGAIN;

  for (idx = 0; idx < max_count && result == NET_SUCCESS; idx++) {
    buffer[idx].person_id = messages[read_start + idx].person_id;
    buffer[idx].index = messages[read_start + idx].index;
    buffer[idx].encryption = messages[read_start + idx].encryption;
    buffer[idx].message = NULL;
    result = util_strcpy(&buffer[idx].message, messages[read_start + idx].message, NET_SUCCESS,
                         NET_ERROR);

    if (person_exists(buffer[idx].person_id)
        && person_encrypt_plain[buffer[idx].encryption][buffer[idx].person_id]) {
      encryptors[buffer[idx].encryption].decode(
          &buffer[idx].message, person_encrypt_key[buffer[idx].encryption][buffer[idx].person_id]);
    }
  }
  *count = idx;
  return result;
}

netResult net_person_count(size_t *list)
{
  size_t person;

  if (is_server < 0) return NET_ERROR;

  *list = 0;
  for (person = 0; person < person_count; person++) {
    if (person_exists(person)) *list += 1;
  }

  return NET_SUCCESS;
}

netResult net_person_list(int list[], size_t limit)
{
  size_t person;

  if (is_server < 0) return NET_ERROR;

  for (person = 0; person < person_count && limit > 0; person++) {
    if (person_exists(person)) {
      *(list++) = person;
      limit--;
    }
  }

  return NET_SUCCESS;
}

static int person_exists(int who)
{
  return who >= 0 && (size_t) who < person_count && person_name[who];
}

static netResult person_make(int who)
{
  int i;
  netResult result;

  if (is_server < 0) return NET_ERROR;
  if (who < 0) return NET_ERROR;
  if (person_exists(who)) return NET_ERROR;


  if ((size_t) who >= person_count) {
    size_t i;
    for (i = 0; i < ENCRYPT_MAX_VAL; i++) {
      person_encrypt_plain[i] =
          realloc(person_encrypt_plain[i], (who + 1) * sizeof(*person_encrypt_plain[i]));
      if (!person_encrypt_plain[i]) return NET_ERROR;
      memset(person_encrypt_plain[i] + person_count, 0,
             (who + 1 - person_count) * sizeof(*person_encrypt_plain[i]));

      person_encrypt_key[i] =
          realloc(person_encrypt_key[i], (who + 1) * sizeof(*person_encrypt_key[i]));
      if (!person_encrypt_key[i]) return NET_ERROR;
      memset(person_encrypt_key[i] + person_count, 0,
             (who + 1 - person_count) * sizeof(*person_encrypt_key[i]));
    }
    person_name = realloc(person_name, (who + 1) * sizeof(*person_name));
    memset(person_name + person_count, 0, (who + 1 - person_count) * sizeof(*person_name));
    person_count = who + 1;
  }


  result = util_strcpy(&person_name[who], "(anon)", NET_SUCCESS, NET_ERROR);

  if (result == NET_SUCCESS)
    for (i = 0; i < ENCRYPT_MAX_VAL; i++) { net_key_set(who, i, ""); }

  return result;
}

static netResult person_free(int who)
{
  size_t i;

  if (is_server < 0) return NET_ERROR;
  if (!person_exists(who)) return NET_ERROR;

  free(person_name[who]);
  person_name[who] = NULL;

  for (i = 0; i < ENCRYPT_MAX_VAL; i++) {
    free(person_encrypt_plain[i][who]);
    person_encrypt_plain[i][who] = NULL;
    encryptors[i].key_free(person_encrypt_key[i][who]);
    person_encrypt_key[i][who] = NULL;
  }

  return NET_SUCCESS;
}

static netResult connection_close(connection_t who)
{
  netResult result;
  if (is_server < 0) return NET_ERROR;
  if (!person_exists(who)) return NET_ERROR;
  result = person_free(who);
  if (result == NET_SUCCESS) result = netio_connection_close(who);
  return result;
}

static netResult messages_set(long int index, long int person_id, long int encryption,
                              const char *message)
{
  netResult result;

  if (is_server < 0) return NET_ERROR;
  if (index < 0) return NET_ERROR;

  if ((size_t) index >= messages_count) {
    messages = realloc(messages, (index + 1) * sizeof(*messages));
    if (!messages) return NET_ERROR;
    memset(messages + messages_count, 0, (index + 1 - messages_count) * sizeof(*messages));

    messages_count = index + 1;
  }

  messages[index].index = index;
  messages[index].person_id = person_id;
  messages[index].encryption = encryption;
  messages[index].message = NULL;
  result = util_strcpy(&messages[index].message, message, NET_SUCCESS, NET_ERROR);

  return result;
}

static netResult broadcast(const net_buffer_t *broadcast)
{
  connection_t client;

  if (is_server != 1) return NET_ERROR;

  for (client = 1; client < person_count; client++)
    if (netio_connection_active(client) && person_exists(client)) {
      if (netio_send(client, broadcast) != NET_SUCCESS) connection_close(client);
    }

  return NET_SUCCESS;
}

static netResult handle_packet_handshake_c(connection_t sender, struct protocol_packet *packet)
{
  netResult result;
  net_buffer_t outgoing = {0};
  struct protocol_packet response = {0};

  if (packet->type != NET_PROTO_HANDSHAKE_C) return NET_ERROR;
  if (is_server != 1) return NET_ERROR;

  response.type = NET_PROTO_HANDSHAKE_S;
  response.as.handshake_s.proto_ver = 0;
  response.as.handshake_s.proto_flags = 0;
  response.as.handshake_s.self_id = sender;

  result = person_make(sender);

  if (result == NET_SUCCESS)
    result = packet_serialize(&outgoing, &response) == PACKET_SUCCESS ? NET_SUCCESS : NET_ERROR;
  if (result == NET_SUCCESS) result = netio_send(sender, &outgoing);

  packet_free(&outgoing);

  return result;
}

static netResult handle_packet_handshake_s(connection_t sender, struct protocol_packet *packet)
{
  netResult result;
  net_buffer_t outgoing = {0};
  struct protocol_packet response = {0};

  if (packet->type != NET_PROTO_HANDSHAKE_S) return NET_ERROR;
  if (is_server) return NET_ERROR;
  if (self_person_id >= 0) return NET_ERROR;

  if (packet->as.handshake_s.proto_ver != 0 || packet->as.handshake_s.proto_flags != 0)
    return NET_ERROR;
  self_person_id = packet->as.handshake_s.self_id;

  response.type = NET_PROTO_INFO_C;
  response.as.info_c.info_type = NET_PINFO_AUDIENCE | NET_PINFO_HISTORY;

  result = packet_serialize(&outgoing, &response) == PACKET_SUCCESS ? NET_SUCCESS : NET_ERROR;
  if (result == NET_SUCCESS) result = netio_send(sender, &outgoing);

  packet_free(&outgoing);

  return result;
}

static netResult handle_packet_info_c(connection_t sender, struct protocol_packet *packet)
{
  netResult result = NET_SUCCESS;
  net_buffer_t outgoing = {0};
  struct protocol_packet response = {0};
  long int idx;

  if (packet->type != NET_PROTO_INFO_C) return NET_ERROR;
  if (is_server != 1) return NET_ERROR;

  if (!(packet->as.info_c.info_type & (NET_PINFO_AUDIENCE | NET_PINFO_HISTORY))) return NET_SUCCESS;

  if (packet->as.info_c.info_type & NET_PINFO_AUDIENCE) {
    for (idx = person_count - 1; idx >= 0 && result == NET_SUCCESS; idx--) {
      if (outgoing.size > (NETIO_BUFFER_MAX_SIZE >> 3)) break;
      if (!person_exists(idx)) continue;

      response.type = NET_PROTO_PERSON;
      response.as.person.person_id = idx;
      response.as.person.name = person_name[idx];
      result = packet_serialize(&outgoing, &response) == PACKET_SUCCESS ? NET_SUCCESS : NET_ERROR;
    }
  }

  if (packet->as.info_c.info_type & NET_PINFO_HISTORY) {
    for (idx = messages_count - 1; idx >= 0 && result == NET_SUCCESS; idx--) {
      if (outgoing.size > (NETIO_BUFFER_MAX_SIZE >> 2)) break;

      response.type = NET_PROTO_MESSAGE;
      response.as.message.person_id = messages[idx].person_id;
      response.as.message.encryption = messages[idx].encryption;
      response.as.message.index = messages[idx].index;
      response.as.message.message = messages[idx].message;
      result = packet_serialize(&outgoing, &response) == PACKET_SUCCESS ? NET_SUCCESS : NET_ERROR;
    }
  }

  if (result == NET_SUCCESS) result = netio_send(sender, &outgoing);

  packet_free(&outgoing);

  return result;
}

static netResult handle_packet_person(connection_t sender, struct protocol_packet *packet)
{
  netResult result = NET_SUCCESS;
  net_buffer_t outgoing = {0};

  if (is_server < 0) return NET_ERROR;
  if (is_server && packet->as.person.person_id != sender) return NET_SUCCESS;

  if (!person_exists(packet->as.person.person_id)) {
    result = person_make(packet->as.person.person_id);
  }

  if (result == NET_SUCCESS) {
    if (person_name[packet->as.person.person_id]) free(person_name[packet->as.person.person_id]);
    result = util_strcpy(&person_name[packet->as.person.person_id], packet->as.person.name,
                         NET_SUCCESS, NET_ERROR);
  }

  if (result == NET_SUCCESS && is_server) {
    result = packet_serialize(&outgoing, packet) == PACKET_SUCCESS ? NET_SUCCESS : NET_ERROR;
    if (result == NET_SUCCESS) result = broadcast(&outgoing);
    packet_free(&outgoing);
  }

  return result;
}

static netResult handle_packet_message(connection_t sender, struct protocol_packet *packet)
{
  netResult result = NET_SUCCESS;
  net_buffer_t outgoing = {0};

  if (is_server < 0) return NET_ERROR;
  if (is_server && packet->as.message.person_id != sender) return NET_SUCCESS;

  if (!person_exists(packet->as.message.person_id)) {
    result = person_make(packet->as.message.person_id);
  }

  if (packet->as.message.index < 0) {
    if (!is_server) return NET_SUCCESS;
    packet->as.message.index = messages_count;
  }

  if (result == NET_SUCCESS) {
    result = messages_set(packet->as.message.index, packet->as.message.person_id,
                          packet->as.message.encryption, packet->as.message.message);
  }

  if (result == NET_SUCCESS && is_server) {
    result = packet_serialize(&outgoing, packet) == PACKET_SUCCESS ? NET_SUCCESS : NET_ERROR;
    if (result == NET_SUCCESS) result = broadcast(&outgoing);
    packet_free(&outgoing);
  }

  return result;
}
