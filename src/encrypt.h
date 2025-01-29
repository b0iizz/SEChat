#ifndef ENCRYPT_H_
#define ENCRYPT_H_

#include <stdlib.h>
#include <string.h>

enum encrypttype {
    ENCRYPT_NONE = 0,
    ENCRYPT_RAIL_FENCE,
    ENCRYPT_CAESAR,
    ENCRYPT_VIGENERE,
    ENCRYPT_ROT13,
    ENCRYPT_ROT47,
    ENCRYPT_ATBASH,
    ENCRYPT_SUBSTITUTION,
    ENCRYPT_PAIRWISE_SUBSTITUTION,
    ENCRYPT_ENIGMA_SINGLE_ROTOR,
    ENCRYPT_ENIGMA,
    ENCRYPT_MAX_VAL
};

typedef struct encryption_type {
    void *(*key_parse)(const char *key);
    void (*key_free)(void *key);
    void (*encode)(char **str, void *key);
    void (*decode)(char **code, void *key);
} encryptor_t;

extern encryptor_t encryptors[ENCRYPT_MAX_VAL];

void encrypt_init();

const char *encrypt_strencryptor(int encryptor);
int encrypt_fencryptor(const char *str);

#endif
