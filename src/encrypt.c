#include "encrypt.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

encryptor_t encryptors[ENCRYPT_MAX_VAL] = { 0 };

static void *encrypt_none_key_parse(const char *key);
static void encrypt_none_key_free(void *key);
static void encrypt_none_encode(char **str, void *key);
static void encrypt_none_decode(char **code, void *key);
static int roll_in_alphabet(int i, int shift, int alphabet_size);

static int roll_in_alphabet(int i, int shift, int alphabet_size);

static void encrypt_rail_fence_encode(char **str, void *key);
static void encrypt_rail_fence_decode(char **code, void *key);

static void *encrypt_caesar_key_parse(const char *key);
static void encrypt_caesar_key_free(void *key);
static void encrypt_caesar_encode(char **str, void *key);
static void encrypt_caesar_decode(char **code, void *key);

static void *encrypt_vigenere_key_parse(const char *key);
static void encrypt_vigenere_key_free(void *key);
static void encrypt_vigenere_encode(char **str, void *key);
static void encrypt_vigenere_decode(char **code, void *key);

static void encrypt_rot13_encode(char **text, void *key);

static void encrypt_rot47_encode(char **text, void *key);

static void encrypt_atbash_encode(char **text, void *key);

static void *encrypt_substitution_key_parse(const char *key);
static void encrypt_substitution_key_free(void *key);
static void encrypt_substitution_encode(char **str, void *key);
static void encrypt_substitution_decode(char **code, void *key);

static void *encrypt_pairwise_substitution_key_parse(const char *key);

enigma_rotor *enigma_rotor_init(const char *name);
static void *encrypt_enigma_single_rotor_key_parse(const char *key);
static void encrypt_enigma_single_rotor_key_free(void *key);
static void encrypt_enigma_single_rotor_encode(char **str, void *key);
static void encrypt_enigma_single_rotor_decode(char **code, void *key);

static void *encrypt_enigma_key_parse(const char *key);
static void encrypt_enigma_key_free(void *key);
static void encrypt_enigma_encode(char **str, void *key);

void encrypt_init()
{
    encryptors[ENCRYPT_NONE].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_NONE].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_NONE].encode = &encrypt_none_encode;
    encryptors[ENCRYPT_NONE].decode = &encrypt_none_decode;

    encryptors[ENCRYPT_RAIL_FENCE].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_RAIL_FENCE].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_RAIL_FENCE].encode = &encrypt_rail_fence_encode;
    encryptors[ENCRYPT_RAIL_FENCE].decode = &encrypt_rail_fence_decode;

    encryptors[ENCRYPT_CAESAR].key_parse = &encrypt_caesar_key_parse;
    encryptors[ENCRYPT_CAESAR].key_free = &encrypt_caesar_key_free;
    encryptors[ENCRYPT_CAESAR].encode = &encrypt_caesar_encode;
    encryptors[ENCRYPT_CAESAR].decode = &encrypt_caesar_decode;

    encryptors[ENCRYPT_VIGENERE].key_parse = &encrypt_vigenere_key_parse;
    encryptors[ENCRYPT_VIGENERE].key_free = &encrypt_vigenere_key_free;
    encryptors[ENCRYPT_VIGENERE].encode = &encrypt_vigenere_encode;
    encryptors[ENCRYPT_VIGENERE].decode = &encrypt_vigenere_decode;

    encryptors[ENCRYPT_ROT13].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_ROT13].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_ROT13].encode = &encrypt_rot13_encode;
    encryptors[ENCRYPT_ROT13].decode = &encrypt_rot13_encode;

    encryptors[ENCRYPT_ROT47].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_ROT47].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_ROT47].encode = &encrypt_rot47_encode;
    encryptors[ENCRYPT_ROT47].decode = &encrypt_rot47_encode;

    encryptors[ENCRYPT_ATBASH].key_parse = &encrypt_none_key_parse;
    encryptors[ENCRYPT_ATBASH].key_free = &encrypt_none_key_free;
    encryptors[ENCRYPT_ATBASH].encode = &encrypt_atbash_encode;
    encryptors[ENCRYPT_ATBASH].decode = &encrypt_atbash_encode;

    encryptors[ENCRYPT_SUBSTITUTION].key_parse = &encrypt_substitution_key_parse;
    encryptors[ENCRYPT_SUBSTITUTION].key_free = &encrypt_substitution_key_free;
    encryptors[ENCRYPT_SUBSTITUTION].encode = &encrypt_substitution_encode;
    encryptors[ENCRYPT_SUBSTITUTION].decode = &encrypt_substitution_decode;

    encryptors[ENCRYPT_PAIRWISE_SUBSTITUTION].key_parse = &encrypt_pairwise_substitution_key_parse;
    encryptors[ENCRYPT_PAIRWISE_SUBSTITUTION].key_free = &encrypt_substitution_key_free;
    encryptors[ENCRYPT_PAIRWISE_SUBSTITUTION].encode = &encrypt_substitution_encode;
    encryptors[ENCRYPT_PAIRWISE_SUBSTITUTION].decode = &encrypt_substitution_decode;

    encryptors[ENCRYPT_ENIGMA_SINGLE_ROTOR].key_parse = &encrypt_enigma_single_rotor_key_parse;
    encryptors[ENCRYPT_ENIGMA_SINGLE_ROTOR].key_free = &encrypt_enigma_single_rotor_key_free;
    encryptors[ENCRYPT_ENIGMA_SINGLE_ROTOR].encode = &encrypt_enigma_single_rotor_encode;
    encryptors[ENCRYPT_ENIGMA_SINGLE_ROTOR].decode = &encrypt_enigma_single_rotor_decode;

    encryptors[ENCRYPT_ENIGMA].key_parse = &encrypt_enigma_key_parse;
    encryptors[ENCRYPT_ENIGMA].key_free = &encrypt_enigma_key_free;
    encryptors[ENCRYPT_ENIGMA].encode = &encrypt_enigma_encode;
    encryptors[ENCRYPT_ENIGMA].decode = &encrypt_enigma_encode;
}

const char *encrypt_strencryptor(int encryptor) {
  switch (encryptor) {
   case ENCRYPT_NONE: return "No Encryption (none)";
   case ENCRYPT_CAESAR: return "Caesar Chiffre (caesar)";
   case ENCRYPT_VIGENERE: return "Vigenere Chiffre (vigenere)";
   case ENCRYPT_ROT13: return "Rot13 Method (rot13)";
   case ENCRYPT_ROT47: return "Rot47 Method (rot47)";
   case ENCRYPT_ATBASH: return "Atbash Encryption (atbash)";
   case ENCRYPT_SUBSTITUTION: return "Substitution (substitution)";
   case ENCRYPT_PAIRWISE_SUBSTITUTION: return "Pairwise Substitution (pair-substitution)";
   case ENCRYPT_ENIGMA_SINGLE_ROTOR: return "Enigma Single Rotor (enigma-rotor)";
   case ENCRYPT_ENIGMA: return "Enigma Encryption (enigma)";
   default: return "Invalid Method!";
  }
}

int encrypt_fencryptor(const char *str) {
  if (!strcmp(str, "none")) return ENCRYPT_NONE;
  if (!strcmp(str, "caesar")) return ENCRYPT_CAESAR;
  if (!strcmp(str, "vigenere")) return ENCRYPT_VIGENERE;
  if (!strcmp(str, "rot13")) return ENCRYPT_ROT13;
  if (!strcmp(str, "rot47")) return ENCRYPT_ROT47;
  if (!strcmp(str, "atbash")) return ENCRYPT_ATBASH;
  if (!strcmp(str, "substitution")) return ENCRYPT_SUBSTITUTION;
  if (!strcmp(str, "pair-substitution")) return ENCRYPT_PAIRWISE_SUBSTITUTION;
  if (!strcmp(str, "enigma-rotor")) return ENCRYPT_ENIGMA_SINGLE_ROTOR;
  if (!strcmp(str, "enigma")) return ENCRYPT_ENIGMA;
  return -1;
}

static int roll_in_alphabet(int i, int shift, int alphabet_size)
{
    if (i < 0)
        return i;
    if (alphabet_size <= 0)
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

static void encrypt_none_encode(char **str, void *key)
{
    (void)str;
    (void)key;
}

static void encrypt_none_decode(char **code, void *key)
{
    (void)code;
    (void)key;
}

/*Rail-Fence-Encryption*/

static void encrypt_rail_fence_encode(char **text, void *key)
{
    int i, j, strl;
    char *str = *text;
    char *code;
    strl = strlen(str);
    code = malloc((strl + 1) * sizeof(char));
    if (code == NULL)
        return;

    j = 0;
    for (i = 0; i < strl; i += 4) {
        code[j] = str[i];
        j++;
    }

    for (i = 1; i < strl; i += 2) {
        code[j] = str[i];
        j++;
    }

    for (i = 2; i < strl; i += 4) {
        code[j] = str[i];
        j++;
    }

    code[strl] = '\0';

    free(*text);
    *text = code;
    (void) key;
}

static void encrypt_rail_fence_decode(char **text, void *key)
{
    int i, j, strl;
    char *code = *text;
    char *str;
    strl = strlen(code);
    str = malloc((strl + 1) * sizeof(char));
    if (str == NULL)
        return;

    j = 0;
    for (i = 0; i < strl; i += 4) {
        str[i] = code[j];
        j++;
    }

    for (i = 1; i < strl; i += 2) {
        str[i] = code[j];
        j++;
    }

    for (i = 2; i < strl; i += 4) {
        str[i] = code[j];
        j++;
    }

    str[strl] = '\0';

    free(str);
    *text = code;
    (void) key;
}

/*Caesar-Cipher*/

static void *encrypt_caesar_key_parse(const char *key)
{
    int i = 0;
    int *keyptr = malloc(sizeof(int));
    if (!keyptr) return NULL;
    *keyptr = atoi(key);
    *keyptr %= 26;
    if (*keyptr == 0) {
        while(key[i] != '\0' && isspace(key[i]))
            i++;
        if (!isdigit(key[i])) {
            free(keyptr);
            return NULL;
         }
    }

    return keyptr;
}

static void encrypt_caesar_key_free(void *key)
{
    free((int *)key);
}

static void encrypt_caesar_encode(char **text, void *key)
{
    int i, let, upper;
    int letshift = *((int *)key);
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++) {
        if (!isalpha(str[i]))
            continue;

        upper = isupper(str[i]);
        let = tolower(str[i]) - 'a';
        let = roll_in_alphabet(let, letshift, 26);
        str[i] = upper ? (let + 'A') : (let + 'a');
    }
}

static void encrypt_caesar_decode(char **codetext, void *key)
{
    int i, let, upper;
    int letshift = (-1) * *((int *)key);
    char *code = *codetext;
    for (i = 0; code[i] != '\0'; i++) {
        if (!isalpha(code[i]))
            continue;

        upper = isupper(code[i]);
        let = tolower(code[i]) - 'a';
        let = roll_in_alphabet(let, letshift, 26);

        code[i] = upper ? (let + 'A') : (let + 'a');
    }
}

/*Vigenere-Cipher*/

static void *encrypt_vigenere_key_parse(const char *key)
{
    int i, str_size = 0;
    int *keyptr;

    for (i = 0; key[i] != '\0'; i++) {
        if (!isalpha(key[i]))
            return NULL;
        str_size++;
    }

    if (!str_size) return NULL;

    keyptr = malloc((1 + str_size) * sizeof(int));
    if (keyptr == NULL)
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

static void encrypt_vigenere_encode(char **text, void *key)
{
    int i, let, upper;
    char *str = *text;
    int letshift, letshift_cnt = 0;
    for (i = 0; str[i] != '\0'; i++) {
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
}

static void encrypt_vigenere_decode(char **codetext, void *key)
{
    int i, let, upper;
    char *code = *codetext;
    int letshift, letshift_cnt = 0;
    for (i = 0; code[i] != '\0'; i++) {
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
}

/*Rot13-Encryption*/

static void encrypt_rot13_encode(char **text, void *key)
{
    int i, let, upper;
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++) {
        if (!isalpha(str[i]))
            continue;

        upper = isupper(str[i]);
        let = tolower(str[i]) - 'a';
        let = roll_in_alphabet(let, 13, 26);
        str[i] = upper ? (let + 'A') : (let + 'a');
    }
    (void)key;
}

/*Rot47-Encryption*/

static void encrypt_rot47_encode(char **text, void *key)
{
    int i;
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] <= 32 ||str[i] == 127)
            continue;

        str[i] = 33 + roll_in_alphabet(str[i] - 33, 47, 94);
    }
    (void)key;
}

/*Atbash-Encryption*/

static void encrypt_atbash_encode(char **text, void *key)
{
    int i;
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++) {
        if (!isalpha(str[i]))
            continue;
        if (isupper(str[i]))
            str[i] = 'Z' - (str[i] - 'A');
        if (islower(str[i]))
            str[i] = 'z' - (str[i] - 'a');
    }
    (void)key;
}

/*Substitution-Cipher*/

static void *encrypt_substitution_key_parse(const char *key)
{
    char c;
    int i;
    char *keyptr;
    for (c = 'a'; c <= 'z'; c++) {
        if (strchr(key, c) == NULL || strchr(key, c) != strrchr(key, c))
            return NULL;
    }

    keyptr = malloc(26 * sizeof(char));

    if (keyptr == NULL)
        return NULL;

    for (i = 0; i < 26; i++)
        keyptr[i] = key[i] - 'a';

    return (void *)keyptr;
}

static void encrypt_substitution_key_free(void *key)
{
    free((char *)key);
}

static void encrypt_substitution_encode(char **text, void *key)
{
    int i, let, upper;
    char *str = *text;
    for (i = 0; str[i] != '\0'; i++) {
        if (!isalpha(str[i]))
            continue;

        upper = isupper(str[i]);
        let = tolower(str[i]) - 'a';
        let = ((char *)key)[let];
        str[i] = upper ? (let + 'A') : (let + 'a');
    }
}

static void encrypt_substitution_decode(char **codetext, void *key)
{
    int i, j, let, upper;
    char *code = *codetext;
    for (i = 0; code[i] != '\0'; i++) {
        if (!isalpha(code[i]))
            continue;

        upper = isupper(code[i]);
        let = tolower(code[i]) - 'a';

        for (j = 0; j < 26; j++) {
            if (((char *)key)[j] == let) {
                let = j;
                break;
            }
        }

        code[i] = upper ? (let + 'A') : (let + 'a');
    }
}

/*Pairwise Substitution*/

static void *encrypt_pairwise_substitution_key_parse(const char *key)
{
    char c;
    int i;
    char *keyptr;
    for (c = 'a'; c <= 'z'; c++) {
        if (strchr(key, c) == NULL || strchr(key, c) != strrchr(key, c))
            return NULL;
    }

    keyptr = malloc(26 * sizeof(char));

    if (keyptr == NULL)
        return NULL;

    for (i = 0; i < 26; i++)
        keyptr[i] = key[i] - 'a';

    for (i = 0; i < 26; i++) {
        if (keyptr[(int) keyptr[i]] != i) {
            free(keyptr);
            return NULL;
        }
    }

    return (void *)keyptr;
}

/*single-Rotor-Enigma*/

enigma_rotor *enigma_rotor_init(const char *name)
{
    enigma_rotor *e_rotor_ptr;
    if (name == NULL)
        return NULL;
    e_rotor_ptr = malloc(sizeof(enigma_rotor));

    if (e_rotor_ptr == NULL)
        return NULL;
    e_rotor_ptr->key = NULL;

    if (0 == strcmp(name,"B")) {
        e_rotor_ptr->key = encrypt_pairwise_substitution_key_parse("yruhqsldpxngokmiebfzcwvjat");
        e_rotor_ptr->turnover_marker = ' ';
    }
    if (0 == strcmp(name,"C")) {
        e_rotor_ptr->key = encrypt_pairwise_substitution_key_parse("fvpjiaoyedrzxwgctkuqsbnmhl");
        e_rotor_ptr->turnover_marker = ' ';
    }
    if (0 == strcmp(name,"I")) {
        e_rotor_ptr->key = encrypt_substitution_key_parse("ekmflgdqvzntowyhxuspaibrcj");
        e_rotor_ptr->turnover_marker = 'q';
    }
    if (0 == strcmp(name,"II")) {
        e_rotor_ptr->key = encrypt_substitution_key_parse("ajdksiruxblhwtmcqgznpyfvoe");
        e_rotor_ptr->turnover_marker = 'e';
    }
    if (0 == strcmp(name,"III")) {
        e_rotor_ptr->key = encrypt_substitution_key_parse("bdfhjlcprtxvznyeiwgakmusqo");
        e_rotor_ptr->turnover_marker = 'v';
    }
    if (0 == strcmp(name,"IV")) {
        e_rotor_ptr->key = encrypt_substitution_key_parse("esovpzjayquirhxlnftgkdcmwb");
        e_rotor_ptr->turnover_marker = 'j';
    }
    if (0 == strcmp(name,"V")) {
        e_rotor_ptr->key = encrypt_substitution_key_parse("vzbrgityupsdnhlxawmjqofeck");
        e_rotor_ptr->turnover_marker = 'z';
    }
    if (e_rotor_ptr->key == NULL) {
        free(e_rotor_ptr);
        return NULL;
    }
    return e_rotor_ptr;
}

static void *encrypt_enigma_single_rotor_key_parse(const char *key)
{
    return (void *) enigma_rotor_init(key);
}

static void encrypt_enigma_single_rotor_key_free(void *key)
{
    enigma_rotor *e_rotor_ptr = (enigma_rotor *)key;
    if (key == NULL)
        return;
    if (e_rotor_ptr->key != NULL)
        free(e_rotor_ptr->key);

    free(e_rotor_ptr);
}

static void encrypt_enigma_single_rotor_encode(char **str, void *key)
{
    enigma_rotor *e_rotor_ptr = (enigma_rotor *)key;
    if (key == NULL)
        return;
    encrypt_substitution_encode(str, e_rotor_ptr->key);
}

static void encrypt_enigma_single_rotor_decode(char **code, void *key)
{
    enigma_rotor *e_rotor_ptr = (enigma_rotor *)key;
    if (key == NULL)
        return;
    encrypt_substitution_decode(code, e_rotor_ptr->key);
}

/*Enigma (multiple rotors)*/

static void *encrypt_enigma_key_parse(const char *key)
{
    int i;
    char *textpart;
    enigma *e_ptr = calloc(1, sizeof(enigma));
    char *key_cpy;
    if (e_ptr == NULL)
        return NULL;

    e_ptr->plugboard_key = NULL;
    e_ptr->rotor_left = NULL;
    e_ptr->rotor_middle = NULL;
    e_ptr->rotor_right = NULL;

    key_cpy = malloc((strlen(key) + 1) *sizeof(char));
    if (key_cpy == NULL)
        return NULL;

    strcpy(key_cpy, key);

    textpart = strtok(key_cpy, ";");
    if (textpart == NULL) {
        encrypt_enigma_key_free(&e_ptr);
        return NULL;
    }
    e_ptr->rotor_left = enigma_rotor_init(textpart);

    textpart = strtok(NULL, ";");
    if (textpart == NULL) {
        encrypt_enigma_key_free(&e_ptr);
        return NULL;
    }
    e_ptr->rotor_middle = enigma_rotor_init(textpart);

    textpart = strtok(NULL, ";");
    if (textpart == NULL) {
        encrypt_enigma_key_free(&e_ptr);
        return NULL;
    }
    e_ptr->rotor_right = enigma_rotor_init(textpart);
    textpart = strtok(NULL, ";");
    if (textpart == NULL) {
        encrypt_enigma_key_free(&e_ptr);
        return NULL;
    }
    e_ptr->reflector = enigma_rotor_init(textpart);
    /*printf("%p", e_ptr->reflector);*/
    if (e_ptr->reflector == NULL || e_ptr->rotor_left == NULL || e_ptr->rotor_middle == NULL || e_ptr->rotor_right == NULL) {
        encrypt_enigma_key_free(&e_ptr);
        return NULL;
    }

    textpart = strtok(NULL, ";");

    if (strlen(textpart) != 3) {
        encrypt_enigma_key_free(&e_ptr);
        return NULL;
    }

    for (i = 0; i < 3; i++) {
        if (!isalpha(textpart[i])) {
            encrypt_enigma_key_free(&e_ptr);
            return NULL;
        }
        e_ptr->starting_position[i] = tolower(textpart[i]) - 'a';
    }

    textpart = strtok(NULL, ";");
    if (textpart == NULL)
        e_ptr->plugboard_key = NULL;
    else
        e_ptr->plugboard_key = encrypt_pairwise_substitution_key_parse(textpart);

    free(key_cpy);
    return e_ptr;
}

static void encrypt_enigma_key_free(void *key)
{
    enigma *e_ptr = (enigma *)key;
    if (key == NULL)
        return;

    if (e_ptr->reflector != NULL)
        free(e_ptr->reflector);
    if (e_ptr->rotor_left != NULL)
        free(e_ptr->rotor_left);
    if (e_ptr->rotor_middle != NULL)
        free(e_ptr->rotor_middle);
    if (e_ptr->rotor_right != NULL)
        free(e_ptr->rotor_right);
    if (e_ptr->plugboard_key != NULL)
        free(e_ptr->plugboard_key);

    free(key);
}

static void encrypt_enigma_encode(char **text, void *key)
{
    int i, cap;
    char *str = *text;
    enigma *e_ptr = (enigma *)key;
    char *let;
    char **passon_str;
    int rotorshift[3];
    /*left - middle - right*/
    let = malloc(2 * sizeof(char));
    let[1] = '\0';
    if (let == NULL)
        return;

    for (i = 0; i < 3; i++)
        rotorshift[i] = e_ptr->starting_position[i];

    for (i = 0; str[i] != '\0'; i++) {
        if (!isalpha(str[i]))
            continue;
        cap = isupper(str[i]);
        let[0] = tolower(str[i]);

        /*step rotors*/
        if (rotorshift[1] + 'a' == e_ptr->rotor_middle->turnover_marker) {
            rotorshift[0] = roll_in_alphabet(rotorshift[0], 1, 26);
            rotorshift[1] = roll_in_alphabet(rotorshift[1], 1, 26);
            rotorshift[2] = roll_in_alphabet(rotorshift[2], 1, 26);
        } else if (rotorshift[2] + 'a' == e_ptr->rotor_right->turnover_marker) {
            rotorshift[1] = roll_in_alphabet(rotorshift[1], 1, 26);
            rotorshift[2] = roll_in_alphabet(rotorshift[2], 1, 26);
        } else {
            rotorshift[2] = roll_in_alphabet(rotorshift[2], 1, 26);
        }

        passon_str = &let;

        /*plugboard*/
        if (e_ptr->plugboard_key != NULL)
            encrypt_substitution_encode(passon_str, (void *)(e_ptr->plugboard_key));

        /*rotors (right to left)*/
        let[0] = 'a' + roll_in_alphabet(let[0]-'a', rotorshift[2], 26);
        encrypt_enigma_single_rotor_encode(passon_str, e_ptr->rotor_right);
        let[0] = 'a' + roll_in_alphabet(let[0]-'a', rotorshift[1] - rotorshift[2], 26);
        encrypt_enigma_single_rotor_encode(passon_str, e_ptr->rotor_middle);
        let[0] = 'a' + roll_in_alphabet(let[0]-'a', rotorshift[0] - rotorshift[1], 26);
        encrypt_enigma_single_rotor_encode(passon_str, e_ptr->rotor_left);
        let[0] = 'a' + roll_in_alphabet(let[0]-'a', - rotorshift[0], 26);

        /*reflector*/
        encrypt_substitution_encode(passon_str, e_ptr->reflector->key);

        /*rotors (left to right)*/
        let[0] = 'a' + roll_in_alphabet(let[0]-'a', rotorshift[0], 26);
        encrypt_enigma_single_rotor_decode(passon_str, e_ptr->rotor_left);
        let[0] = 'a' + roll_in_alphabet(let[0]-'a', rotorshift[1] - rotorshift[0], 26);
        encrypt_enigma_single_rotor_decode(passon_str, e_ptr->rotor_middle);
        let[0] = 'a' + roll_in_alphabet(let[0]-'a', rotorshift[2] - rotorshift[1], 26);
        encrypt_enigma_single_rotor_decode(passon_str, e_ptr->rotor_right);
        let[0] = 'a' + roll_in_alphabet(let[0]-'a', - rotorshift[2], 26);

        /*plugboard*/
        if (e_ptr->plugboard_key != NULL)
            encrypt_substitution_decode(passon_str, e_ptr->plugboard_key);

       str[i] = let[0];
       if (cap)
            str[i] = str[i] - 'a' + 'A';
    }
    free(let);
}
