#ifndef NET_H_
#define NET_H_

#include "encrypt.h"

typedef int netResult;

#define NET_MYSELF -1

enum netresults { NET_SUCCESS = 0, NET_TRY_AGAIN = 1, NET_ERROR = -1 };
enum netflags { NET_FHISTORY = 1 };

struct net_message {
    long int index;
    long int person_id;
    long int encryption;
    char *message;
};

netResult net_init();
netResult net_exit();

netResult net_connect(const char *hostname, const char *port);
netResult net_serve(const char *port);
netResult net_reset();

netResult net_tick();

netResult net_name_set(int person, const char *name);
netResult net_name_get(int person, char **name);

netResult net_key_set(int person, int method, const char *key);
netResult net_key_get(int person, int method, char **dst);

netResult net_message_send(int encryption, const char *message);
netResult net_message_recv(struct net_message *buffer, size_t *count,
                           size_t limit, int flags);

netResult net_messages_decoding_set(int enabled);

netResult net_person_count(size_t *list);
netResult net_person_list(int list[], size_t limit);

#endif /*NET_H_*/
