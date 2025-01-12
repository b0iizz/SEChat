#include "encrypt.h"
#include <ctype.h>
#include <stdlib.h>

encryptor_t encryptors[ENCRYPT_MAX_VAL] = { 0 };

static void *encrypt_none_key_parse(const char *key);
static void encrypt_none_key_free(void *key);
static void *encrypt_none_state_alloc();
static void encrypt_none_state_free(void *state);
static void encrypt_none_encode(char **str, void *key, void *state);
static void encrypt_none_decode(char **code, void *key, void *state);

static int roll_in_alphabet(int i, int shift, int alphabet_size);

static void *encrypt_caesar_key_parse(const char *key);
static void encrypt_caesar_key_free(void *key);

static void encrypt_caesar_encode(char **str, void *key, void *state);
static void encrypt_caesar_decode(char **code, void *key, void *state);

static void *encrypt_vigenere_key_parse(const char *key);
static void encrypt_vigenere_key_free(void *key);
static void encrypt_vigenere_encode(char **str, void *key, void *state);
static void encrypt_vigenere_decode(char **code, void *key, void *state);

static void encrypt_rot13_encode(char **text, void *key, void *state);

static void encrypt_rot47_encode(char **text, void *key, void *state);

static void encrypt_atbash_encode(char **text, void *key, void *state);

static void *encrypt_substitution_key_parse(const char *key);
static void encrypt_substitution_key_free(void *key);
static void encrypt_substitution_encode(char **str, void *key, void *state);
static void encrypt_substitution_decode(char **code, void *key, void *state);

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
    encryptors[ENCRYPT_CAESAR].state_alloc = &encrypt_none_state_alloc;
    encryptors[ENCRYPT_CAESAR].state_free = &encrypt_none_state_free;
    encryptors[ENCRYPT_CAESAR].encode = &encrypt_caesar_encode;
    encryptors[ENCRYPT_CAESAR].decode = &encrypt_caesar_decode;

    encryptors[ENCRYPT_VIGENERE].key_parse = &encrypt_vigenere_key_parse;
    encryptors[ENCRYPT_VIGENERE].key_free = &encrypt_vigenere_key_free;
    encryptors[ENCRYPT_VIGENERE].state_alloc = &encrypt_none_state_alloc;
    encryptors[ENCRYPT_VIGENERE].state_free = &encrypt_none_state_free;
    encryptors[ENCRYPT_VIGENERE].encode = &encrypt_vigenere_encode;
    encryptors[ENCRYPT_VIGENERE].decode = &encrypt_vigenere_decode;

    encryptors[ENCRYPT_ROT13].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_ROT13].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_ROT13].state_alloc = &encrypt_none_state_alloc;
    encryptors[ENCRYPT_ROT13].state_free = &encrypt_none_state_free;
    encryptors[ENCRYPT_ROT13].encode = &encrypt_rot13_encode;
    encryptors[ENCRYPT_ROT13].decode = &encrypt_rot13_encode;

    encryptors[ENCRYPT_ROT47].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_ROT47].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_ROT47].state_alloc = &encrypt_none_state_alloc;
    encryptors[ENCRYPT_ROT47].state_free = &encrypt_none_state_free;
    encryptors[ENCRYPT_ROT47].encode = &encrypt_rot47_encode;
    encryptors[ENCRYPT_ROT47].decode = &encrypt_rot47_encode;

    encryptors[ENCRYPT_ATBASH].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_ATBASH].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_ATBASH].state_alloc = &encrypt_none_state_alloc;
    encryptors[ENCRYPT_ATBASH].state_free = &encrypt_none_state_free;
    encryptors[ENCRYPT_ATBASH].encode = &encrypt_atbash_encode;
    encryptors[ENCRYPT_ATBASH].decode = &encrypt_atbash_encode;

    encryptors[ENCRYPT_SUBSTITUTION].key_parse = &encrypt_substitution_key_parse;
    encryptors[ENCRYPT_SUBSTITUTION].key_free = &encrypt_substitution_key_free;
    encryptors[ENCRYPT_SUBSTITUTION].state_alloc = &encrypt_none_state_alloc;
    encryptors[ENCRYPT_SUBSTITUTION].state_free = &encrypt_none_state_free;
    encryptors[ENCRYPT_SUBSTITUTION].encode = &encrypt_substitution_encode;
    encryptors[ENCRYPT_SUBSTITUTION].decode = &encrypt_substitution_decode;
}

static int roll_in_alphabet(int i, int shift, int alphabet_size)
{
    if(i < 0)
        return i;
    if(alphabet_size <= 0)
        return 0;
    i += shift;
    while (i < 0)
        i += alphabet_size;

    return i % alphabet_size;
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
static void encrypt_none_encode(char **str, void *key, void *state)
{
    (void)str;
    (void)key;
    (void)state;
}
static void encrypt_none_decode(char **code, void *key, void *state)
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
    if(keyptr = NULL)
        return NULL;

    *keyptr = atoi(key);
    *keyptr %= 26;
    if(*keyptr == 0){
        while(key[i] != '\0' && isspace(key[i]))
            i++;
        if(!isdigit(key[i]))
            return NULL;
    }

    return keyptr;
}
static void encrypt_caesar_key_free(void *key)
{
    free((int *)key);
}

static void encrypt_caesar_encode(char **text, void *key, void *state)
{
    int i, let, upper;
    int letshift = *((int *)key);
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (!isalpha(str[i]))
            continue;

        upper = isupper(str[i]);
        let = tolower(str[i]) - 'a';
        let = roll_in_alphabet(let, letshift, 26);
        str[i] = upper ? (let + 'A') : (let + 'a');
    }
    (void)state;
}
static void encrypt_caesar_decode(char **codetext, void *key, void *state)
{
    int i, let, upper;
    int letshift = (-1) * *((int *)key);
    char *code = *codetext;
    for (i = 0; code[i] != '\0'; i++)
    {
        if (!isalpha(code[i]))
            continue;

        upper = isupper(code[i]);
        let = tolower(code[i]) - 'a';
        let = roll_in_alphabet(let, letshift, 26);

        code[i] = upper ? (let + 'A') : (let + 'a');
    }
    (void)state;
}

/*Vigenere-Cipher*/

static void *encrypt_vigenere_key_parse(const char *key)
{
    int i, str_size = 0;
    int *keyptr;

    for (i = 0; key[i] != '\0'; i++)
    {
        if(!isalpha(key[i]))
            return NULL;
        str_size++;
    }

    keyptr = malloc((1 + str_size) * sizeof(int));
    if(keyptr = NULL)
        return NULL;

    keyptr[0] = str_size;

    for (i = 0; i < str_size; i++)
        keyptr[1 + i] = tolower(key[i]) - 'a';

    return keyptr;
}
static void encrypt_vigenere_key_free(void *key)
{
    free((int *)key);
}
static void encrypt_vigenere_encode(char **text, void *key, void *state)
{
    int i, let, upper;
    char *str = *text;
    int letshift, letshift_cnt = 0;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (!isalpha(str[i]))
            continue;

        letshift = ((int *)key)[1 + letshift_cnt];

        upper = isupper(str[i]);
        let = tolower(str[i]) - 'a';
        let = roll_in_alphabet(let, letshift, 26);
        str[i] = upper ? (let + 'A') : (let + 'a');

        letshift_cnt++;
        letshift_cnt %= *((int *)key);
    }
    (void)state;
}
static void encrypt_vigenere_decode(char **codetext, void *key, void *state)
{
    int i, let, upper;
    char *code = *codetext;
    int letshift, letshift_cnt = 0;
    for (i = 0; code[i] != '\0'; i++)
    {
        if (!isalpha(code[i]))
            continue;

        letshift = (-1) * ((int *)key)[1 + letshift_cnt];

        upper = isupper(code[i]);
        let = tolower(code[i]) - 'a';
        let = roll_in_alphabet(let, letshift, 26);
        code[i] = upper ? (let + 'A') : (let + 'a');

        letshift_cnt++;
        letshift_cnt %= *((int *)key);
    }
    (void)state;
}

/*Rot13-Encryption*/

static void encrypt_rot13_encode(char **text, void *key, void *state)
{
    int i, let, upper;
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (!isalpha(str[i]))
            continue;

        upper = isupper(str[i]);
        let = tolower(str[i]) - 'a';
        let = roll_in_alphabet(let, 13, 26);
        str[i] = upper ? (let + 'A') : (let + 'a');
    }
    (void)key;
    (void)state;
}

/*Rot47-Encryption*/

static void encrypt_rot47_encode(char **text, void *key, void *state)
{
    int i;
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (str[i] <= 32 ||str[i] == 127)
            continue;

        str[i] = 33 + roll_in_alphabet(str[i] - 33, 47, 94);
    }
    (void)key;
    (void)state;
}

/*Atbash-Encryption*/

static void encrypt_atbash_encode(char **text, void *key, void *state)
{
    int i;
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (!isalpha(str[i]))
            continue;
        if(isupper(str[i]))
            str[i] = 'Z' - (str[i] - 'A');
        if(islower(str[i]))
            str[i] = 'z' - (str[i] - 'a');
    }
    (void)key;
    (void)state;
}

/*Substitution-Cipher*/

static void *encrypt_substitution_key_parse(const char *key)
{
    char c;
    int i;
    char *keyptr;
    for (c = 'a'; c <= 'z'; c++)
    {
        if(strchr(key, c) == NULL || strchr(key, c) != strrchr(key, c))
            return NULL;
    }

    keyptr = malloc(26 * sizeof(char));

    if(keyptr == NULL)
        return NULL;

    for(i = 0; i < 26; i++)
        keyptr[i] = key[i] - 'a';

    return (void *)keyptr;
}
static void encrypt_substitution_key_free(void *key)
{
    free((char *)key);
}
static void encrypt_substitution_encode(char **text, void *key, void *state)
{
    int i, let, upper;
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (!isalpha(str[i]))
            continue;

        upper = isupper(str[i]);
        let = tolower(str[i]) - 'a';
        let = ((char *)key)[let];
        str[i] = upper ? (let + 'A') : (let + 'a');
    }
    (void)state;
}
static void encrypt_substitution_decode(char **codetext, void *key, void *state)
{
    int i, j, let, upper;
    char *code = *codetext;
    for (i = 0; code[i] != '\0'; i++)
    {
        if (!isalpha(code[i]))
            continue;

        upper = isupper(code[i]);
        let = tolower(code[i]) - 'a';

        for (j = 0; i < 26; j++)
        {
            if(((char *)key)[j]==let){
                let = j;
                break;
            }
        }

        code[i] = upper ? (let + 'A') : (let + 'a');
    }
    (void)state;
}
