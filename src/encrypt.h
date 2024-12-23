#ifndef ENCRYPT_H_
#define ENCRYPT_H_

#include <stdlib.h>
#include <string.h>

enum encrypttype { ENCRYPT_NONE = 0, ENCRYPT_MAX_VAL };

typedef struct encryption_type {
  void *(*key_parse)(const char *key);
  void (*key_free)(void *key);
  void *(*state_alloc)();
  void (*state_free)(void *state);
  void (*encode)(char *str, void *key, void *state);
  void (*decode)(char *code, void *key, void *state);
} encryptor_t;

extern encryptor_t encryptors[ENCRYPT_MAX_VAL];

void encrypt_init();

#endif
