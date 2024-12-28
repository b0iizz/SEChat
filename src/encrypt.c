#include "encrypt.h"

encryptor_t encryptors[ENCRYPT_MAX_VAL] = { 0, ENCRYPT_CAESAR};

static void *encrypt_none_key_parse(const char *key);
static void encrypt_none_key_free(void *key);
static void *encrypt_none_state_alloc();
static void encrypt_none_state_free(void *state);
static void encrypt_none_encode(char *str, void *key, void *state);
static void encrypt_none_decode(char *code, void *key, void *state);

static void *encrypt_caesar_key_parse(const char *key);
static void encrypt_caesar_key_free(void *key);
static void *encrypt_caesar_state_alloc();
static void encrypt_caesar_state_free(void *state);
static void encrypt_caesar_encode(char *str, void *key, void *state);
static void encrypt_caesar_decode(char *code, void *key, void *state);

void encrypt_init()
{
    encryptors[ENCRYPT_NONE].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_NONE].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_NONE].state_alloc = &encrypt_none_state_alloc;
    encryptors[ENCRYPT_NONE].state_free = &encrypt_none_state_free;
    encryptors[ENCRYPT_NONE].encode = &encrypt_none_encode;
    encryptors[ENCRYPT_NONE].decode = &encrypt_none_decode;

    encryptors[ENCRYPT_CAESAR].key_parse = &encrypt_caesar_key_parse;
    encryptors[ENCRYPT_CAESAR].key_free = &encrypt_caesar_key_free;
    encryptors[ENCRYPT_CAESAR].state_alloc = &encrypt_caesar_state_alloc;
    encryptors[ENCRYPT_CAESAR].state_free = &encrypt_caesar_state_free;
    encryptors[ENCRYPT_CAESAR].encode = &encrypt_caesar_encode;
    encryptors[ENCRYPT_CAESAR].decode = &encrypt_caesar_decode;
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

/*Caesar-Cipher*/

static void *encrypt_caesar_key_parse(const char *key)
{
    int i = 0;
    int *keyptr = malloc(sizeof(int));
    *keyptr = atoi(key);
    if(*keyptr == 0){
        while(key[i] != '\0' && isspace(key[i]))
            i++;
        if(!isdigit(key[i]))
            return NULL;
    }

    return &keyptr;
}
static void encrypt_caesar_key_free(void *key)
{
    free((int *)key);
}
static void *encrypt_caesar_state_alloc()
{
    return &encryptors; /*Non NULL address*/
}
static void encrypt_caesar_state_free(void *state)
{
    (void)state;
}
static void encrypt_caesar_encode(char *str, void *key, void *state)
{
    int i;
    int letshift = *((int *)key);
    for (i = 0; str[i] != '\0'; i++)
    {
        if (!isletter(str[i]))
            continue;

        if (isupper(str[i])) {
            str[i] += letshift;
            if(str[i]>'Z')
                str[i] -= 26;
        } else {
            str[i] += letshift;
            if(str[i]>'z')
                str[i] -= 26;
        }
    }

}
static void encrypt_caesar_decode(char *code, void *key, void *state)
{
    int i;
    int letshift = *((int *)key);
    for (i = 0; code[i] != '\0'; i++)
    {
        if (!isletter(code[i]))
            continue;

        if (isupper(code[i])) {
            code[i] -= letshift;
            if(code[i] < 'A')
                code[i] += 26;
        } else {
            code[i] -= letshift;
            if(code[i] < 'a')
                code[i] += 26;
        }
    }
}
