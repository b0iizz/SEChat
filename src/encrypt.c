#include "encrypt.h"

encryptor_t encryptors[ENCRYPT_MAX_VAL] = { 0 };

static void *encrypt_none_key_parse(const char *key);
static void encrypt_none_key_free(void *key);
static void *encrypt_none_state_alloc();
static void encrypt_none_state_free(void *state);
static void encrypt_none_encode(char *str, void *key, void *state);
static void encrypt_none_decode(char *code, void *key, void *state);

void encrypt_init()
{
    encryptors[ENCRYPT_NONE].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_NONE].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_NONE].state_alloc = &encrypt_none_state_alloc;
    encryptors[ENCRYPT_NONE].state_free = &encrypt_none_state_free;
    encryptors[ENCRYPT_NONE].encode = &encrypt_none_encode;
    encryptors[ENCRYPT_NONE].decode = &encrypt_none_decode;
}

/*Do not encrypt data*/

static void *encrypt_none_key_parse(const char *key)
{
    (void)key;
    return &encryptors; /*Non NULL adress*/
}
static void encrypt_none_key_free(void *key)
{
    (void)key;
}
static void *encrypt_none_state_alloc()
{
    return &encryptors; /*Non NULL address*/
}
static void encrypt_none_state_free(void *state)
{
    (void)state;
}
static void encrypt_none_encode(char *str, void *key, void *state)
{
    (void)str;
    (void)key;
    (void)state;
}
static void encrypt_none_decode(char *code, void *key, void *state)
{
    (void)code;
    (void)key;
    (void)state;
}
