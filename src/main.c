#include <stdio.h>

#include "interface.h"
#include "net.h"
#include "util.h"
#include <string.h>

static int handle_arguments(int argc, char **argv);
static int parse_encryption(const char *encryptor);
static void display_help(char **argv);
static void command_connect(char **argv);
static void command_serve(char **argv);
static void command_key(char **argv);
static void command_encrypt(char **argv, int *encryption);
static void command_name(char **argv);
static int handle_command(const char *message, int *loop, int *encryption);
static int handle_net_message(struct net_message *buffer);

int main(int argc, char **argv)
{
  int loop = 1;
  int encryption = ENCRYPT_NONE;

  encrypt_init();

  if (net_init() != NET_SUCCESS) return -1;

  if (interface_init() != UI_SUCCESS) return -1;

  if (argc < 1) return 1;

  if (argc > 1) {
    if (!handle_arguments(argc, argv)) { net_reset(); }
  }

  if (interface_status("SEChat", "!help - !quit") != UI_SUCCESS) return -1;

  while (loop) {
    uiResult status;
    netResult netStatus;

    const char *input;
    struct net_message buffer[1];
    size_t netCount = 0;

    if (interface_tick() != UI_SUCCESS) return -1;
    if (net_tick() != NET_SUCCESS) {
      net_reset();
      interface_message_send("error in net!");
    }

    while ((status = interface_message_recv(&input)) == UI_SUCCESS) {
      if (handle_command(input, &loop, &encryption)) continue;
      if (net_message_send(encryption, input) != NET_SUCCESS) {
        interface_message_send("### Could not send message!");
      }
    }
    if (status == UI_ERROR) {
      /*There has been an error in the ui*/
      return 1;
    }

    while ((netStatus = net_message_recv(buffer, &netCount, sizeof(buffer) / sizeof(*buffer), 0))
           == NET_SUCCESS) {
      if (!netCount) break;
      if (!handle_net_message(buffer)) { interface_message_send("### Could not display message!"); }
    }

    if (netStatus == NET_ERROR) { return 2; }
  }

  if (net_exit() != UI_SUCCESS) return -1;
  if (interface_exit() != UI_SUCCESS) return -1;
  return 0;
}

static int handle_arguments(int argc, char **argv)
{
  int idx;

  const char *ip = "127.0.0.1";
  const char *port = "10001";

  for (idx = 2; idx < argc; idx++) {
    if (util_startswith(argv[idx], "ip=")) { ip = argv[idx] + strlen("ip="); }
    if (util_startswith(argv[idx], "port=")) { port = argv[idx] + strlen("port="); }
  }

  if (!strcmp(argv[1], "serve")) {
    interface_message_send("Listening on port:");
    interface_message_send(port);
    if (net_serve(port) != NET_SUCCESS) return 0;
  } else if (!strcmp(argv[1], "connect")) {
    interface_message_send("Connecting on ip:");
    interface_message_send(ip);
    interface_message_send("And on port:");
    interface_message_send(port);
    if (net_connect(ip, port) != NET_SUCCESS) return 0;
  } else
    return 0;
  return 1;
}

static int handle_command(const char *message, int *loop, int *encryption)
{
  char *copy = NULL;
  char **argv = NULL;
  if (strlen(message) < 1 || *message != '!') return 0;
  ++message;
  if (!util_strcpy(&copy, message, 1, 0)) return 1;
  if (!util_split(&argv, copy)) {
    free(copy);
    return 1;
  }
  if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "q")) {
    *loop = 0;
  } else if (!strcmp(argv[0], "help") || !strcmp(argv[0], "h")) {
    display_help(argv);
  } else if (!strcmp(argv[0], "top") || !strcmp(argv[0], "t")) {
    interface_scroll_set(32767);
  } else if (!strcmp(argv[0], "bottom") || !strcmp(argv[0], "b")) {
    interface_scroll_set(0);
  } else if (!strcmp(argv[0], "up") || !strcmp(argv[0], "u")) {
    interface_scroll_relative(0, 1);
  } else if (!strcmp(argv[0], "down") || !strcmp(argv[0], "d")) {
    interface_scroll_relative(0, -1);
  } else if (!strcmp(argv[0], "clear") || !strcmp(argv[0], "cl")) {
    interface_message_clear();
  } else if (!strcmp(argv[0], "connect") || !strcmp(argv[0], "c")) {
    command_connect(argv);
  } else if (!strcmp(argv[0], "serve") || !strcmp(argv[0], "s")) {
    command_serve(argv);
  } else if (!strcmp(argv[0], "key") || !strcmp(argv[0], "k")) {
    command_key(argv);
  } else if (!strcmp(argv[0], "name") || !strcmp(argv[0], "n")) {
    command_name(argv);
  } else if (!strcmp(argv[0], "encrypt") || !strcmp(argv[0], "e")) {
    command_encrypt(argv, encryption);
  }
  free(argv);
  free(copy);
  return 1;
}

static int parse_encryption(const char *encryptor)
{
  if (!strcmp(encryptor, "caesar")) return ENCRYPT_CAESAR;
  if (!strcmp(encryptor, "vigenere")) return ENCRYPT_VIGENERE;
  if (!strcmp(encryptor, "rot13")) return ENCRYPT_ROT13;
  if (!strcmp(encryptor, "rot47")) return ENCRYPT_ROT47;
  if (!strcmp(encryptor, "atbash")) return ENCRYPT_ATBASH;
  if (!strcmp(encryptor, "substitution")) return ENCRYPT_SUBSTITUTION;
  if (!strcmp(encryptor, "pair-substitution")) return ENCRYPT_PAIRWISE_SUBSTITUTION;
  return ENCRYPT_NONE;
}

static void display_help(char **argv)
{
  int idx;
  for (idx = 1; argv[idx]; idx++) {
    if (!strcmp(argv[idx], "connect")) {
      interface_message_send(
          "!connect [ip=###] [port=###]\n"
          "Connect to ip[ip] on port[port]\n"
          "Defaults:\n"
          "  ip=127.0.0.1\n"
          "  port=10001");
    } else if (!strcmp(argv[idx], "serve")) {
      interface_message_send(
          "!serve [port=###]\n"
          "Serve clients on port[port]\n"
          "Defaults:\n"
          "  port=10001");
    } else if (!strcmp(argv[idx], "key")) {
      interface_message_send(
          "!key [key=###] [encrypt=###] [name=###]\n"
          "Set key[key] of method[encrypt] on person[name]\n"
          "Defaults:\n"
          "  key="
          "\n"
          "  encrypt=none\n"
          "  name=(you)");
    } else if (!strcmp(argv[idx], "name")) {
      interface_message_send(
          "!name [name=###]\n"
          "Set your own name\n"
          "Defaults:\n"
          "  name=(empty string)");
    } else if (!strcmp(argv[idx], "encrypt")) {
      interface_message_send(
          "!encrypt [encrypt=###]\n"
          "Set your own encryption\n"
          "Defaults:\n"
          "  encrypt=none\n"
          "Possible values:\n"
          "  none\n"
          "  caesar\n"
          "  vigenere\n"
          "  rot13\n"
          "  rot47\n"
          "  atbash\n"
          "  substitution\n"
          "  pair-substitution");
    }
  }
  if (!(idx - 1))
    interface_message_send(
        "!quit    - !q:  Quit the application\n"
        "!help    - !h:  Display this help message\n"
        "!top     - !t:  Jump to top of messages\n"
        "!bottom  - !b:  Jump to bottom of messages\n"
        "!up      - !u:  Scroll one page upwards\n"
        "!down    - !d:  Scroll one page downwards\n"
        "!clear   - !cl: Clear all messages\n"
        "!connect :      Connect to server\n"
        "!serve :        Serve clients\n"
        "!key     - !k:  Set key of person\n"
        "!name    - !n:  Set own name\n"
        "!encrypt - !e:  Set own encryption method");
}

static void command_connect(char **argv)
{
  int idx;
  const char *ip = "127.0.0.1";
  const char *port = "10001";
  for (idx = 1; argv[idx]; idx++) {
    if (util_startswith(argv[idx], "ip=")) { ip = argv[idx] + strlen("ip="); }
    if (util_startswith(argv[idx], "port=")) { port = argv[idx] + strlen("port="); }
  }
  net_reset();
  interface_message_clear();
  interface_message_send("Connecting on ip:");
  interface_message_send(ip);
  interface_message_send("And on port:");
  interface_message_send(port);
  net_connect(ip, port);
}

static void command_serve(char **argv)
{
  int idx;
  const char *port = "10001";
  for (idx = 1; argv[idx]; idx++) {
    if (util_startswith(argv[idx], "port=")) { port = argv[idx] + strlen("port="); }
  }
  net_reset();
  interface_message_clear();
  interface_message_send("Listening on port:");
  interface_message_send(port);
  net_serve(port);
}

static void command_encrypt(char **argv, int *encryption)
{
  int idx;
  for (idx = 1; argv[idx]; idx++) {
    if (util_startswith(argv[idx], "encrypt=")) {
      const char *encrypt_name = argv[idx] + strlen("encrypt=");
      *encryption = parse_encryption(encrypt_name);
    }
  }
}

static void command_name(char **argv)
{
  int idx;
  const char *name = "";
  for (idx = 1; argv[idx]; idx++) {
    if (util_startswith(argv[idx], "name=")) { name = argv[idx] + strlen("name="); }
  }
  net_name_set(NET_MYSELF, name);
}

static void command_key(char **argv)
{
  unsigned int idx;
  const char *name = NULL;
  int encryption = ENCRYPT_NONE;
  const char *key = NULL;
  long int person = NET_MYSELF;

  struct net_message buffer[80];
  size_t num_msgs = 0;

  for (idx = 1; argv[idx]; idx++) {
    if (util_startswith(argv[idx], "name=")) { name = argv[idx] + strlen("name="); }
    if (util_startswith(argv[idx], "key=")) { key = argv[idx] + strlen("key="); }
    if (util_startswith(argv[idx], "encrypt=")) {
      encryption = parse_encryption(argv[idx] + strlen("encrypt="));
    }
  }

  if (name) {
    size_t i;
    size_t person_count;
    int *people;
    if (net_person_count(&person_count) != NET_SUCCESS) return;
    people = malloc(person_count * sizeof(*people));
    if (net_person_list(people, person_count) != NET_SUCCESS) {
      free(people);
      return;
    }
    for (i = 0; i < person_count; i++) {
      char *pname;
      if (net_name_get(people[i], &pname) != NET_SUCCESS) continue;
      if (!strcmp(pname, name)) person = people[i];
      free(pname);
    }
    free(people);
    if (person == NET_MYSELF) {
      interface_message_send("Could not find person with the specified name!");
      return;
    }
  }

  if (key) {
    net_key_set(person, encryption, key);
    interface_message_clear();
    net_message_recv(buffer, &num_msgs, 80, NET_FHISTORY);
    for (idx = 0; idx < num_msgs; idx++) { handle_net_message(buffer + idx); }
  } else {
    char *str = NULL;
    if (net_key_get(person, encryption, &str) == NET_SUCCESS && str) {
      interface_message_send("Person has the following saved key:");
      interface_message_send(str);
      free(str);
    }
  }
}

static int handle_net_message(struct net_message *buffer)
{
  char tmp_buf[128];
  char *name;
  if (net_name_get(buffer->person_id, &name) != NET_SUCCESS) { name = NULL; }
  sprintf(tmp_buf, "%.80s said (with index %li):", name ? name : "Unknown User", buffer->index);
  interface_message_send(tmp_buf);
  interface_message_send(buffer->message);

  free(buffer->message);
  free(name);
  return 1;
}
