#ifndef NET_H_
#define NET_H_

#include "packet.h"
#include "socketxp.h"
#include "encrypt.h"

typedef int netResult;
typedef int connection_t;

enum netresults {
  NET_SUCCESS = 0,
  NET_TRY_AGAIN = 1,
  NET_ERROR = -1
};

typedef struct persona_t {
  short int identifier;
  const char *name;
  short int encryptor_id;
  void *key;
}

netResult net_init();
netResult net_exit();

netResult net_myself_rename(const char *name);
netResult net_myself_encrypt(short int encryptor_id, const char *key);

netResult net_connect(const char *hostname, const char *port);
netResult net_serve();
netResult net_reset();

netResult net_tick();

netResult net_personae_list(const persona_t **dst);
netResult net_personae_decrypt(short int identifier, const char *key);

netResult net_message_recv(char **message);
netResult net_message_send(const char *message);

#endif /*NET_H_*/
