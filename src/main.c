#include <stdio.h>

#include "interface.h"
#include "net.h"
#include "util.h"
#include <string.h>

static int handle_arguments(int argc, char **argv);
static int handle_command(const char *message, int *loop);
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
      if (handle_command(input, &loop)) continue;
      if (net_message_send(encryption, input) != NET_SUCCESS) {
        interface_message_send("### Could not send message!");
      }
    }
    if (status == UI_ERROR) {
      /*There has been an error in the ui*/
      return 1;
    }

    while ((netStatus = net_message_recv(buffer, &netCount, sizeof(buffer)/sizeof(*buffer), 0)) == NET_SUCCESS) {
      if (!netCount) break;
      if (!handle_net_message(buffer)) {
        interface_message_send("### Could not display message!");
      }
    }

    if (netStatus == NET_ERROR) {
      return 2;
    }

  }

  if (interface_exit() != UI_SUCCESS) return -1;
  if (net_exit() != UI_SUCCESS) return -1;
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

static int handle_command(const char *message, int *loop)
{
  if (strlen(message) < 1 || *message != '!') return 0;
  ++message;
  if (!strcmp(message, "quit") || !strcmp(message, "q")) {
    *loop = 0;
  } else if (!strcmp(message, "help") || !strcmp(message, "h")) {
    interface_message_send(
        "!quit    - !q:  Quit the application\n"
        "!help    - !h:  Display this help message\n"
        "!top     - !t:  Jump to top of messages\n"
        "!bottom  - !b:  Jump to bottom of messages\n"
        "!up      - !u:  Scroll one page upwards\n"
        "!down    - !d:  Scroll one page downwards\n"
        "!clear   - !cl: Clear all messages");
  } else if (!strcmp(message, "top") || !strcmp(message, "t")) {
    interface_scroll_set(32767);
  } else if (!strcmp(message, "bottom") || !strcmp(message, "b")) {
    interface_scroll_set(0);
  } else if (!strcmp(message, "up") || !strcmp(message, "u")) {
    interface_scroll_relative(0, 1);
  } else if (!strcmp(message, "down") || !strcmp(message, "d")) {
    interface_scroll_relative(0, -1);
  } else if (!strcmp(message, "clear") || !strcmp(message, "cl")) {
    interface_message_clear();
  }
  return 1;
}

static int handle_net_message(struct net_message *buffer) {
  char tmp_buf[128];
  char *name;
  if (net_name_get(buffer->person_id, &name) != NET_SUCCESS) {
    name = NULL;
  }
  sprintf(tmp_buf, "%.80s said (with index %li):", name ? name : "Unknown User", buffer->index);
  interface_message_send(tmp_buf);
  interface_message_send(buffer->message);

  free(buffer->message);
  free(name);
  return 1;
}
