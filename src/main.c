#include <stdio.h>

#include "interface.h"
/*
#include "socketxp.h"
*/
#include <string.h>

/*
int sockettest();
*/
int uitest();

static int handle_command(const char *message, int *loop)
{
    if (strlen(message) < 1 || *message != '!')
        return 0;
    ++message;
    if (!strcmp(message, "quit") || !strcmp(message, "q")) {
        *loop = 0;
    } else if (!strcmp(message, "help") || !strcmp(message, "h")) {
        interface_message_send("!quit    - !q:  Quit the application\n"
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

int main(void)
{
    return uitest();
}
/*
int sockettest()
{
  sxp_t socket;
  sxpResult status;
  if ((status = sxp_init()) != SXP_SUCCESS) {
    printf("Error in initialization of socket abstraction layer! %i\n", status);
    return 1;
  }
  printf("sxp_init successful!\n");

  if ((status = sxp_create(&socket, AF_INET, SOCK_STREAM, 0, SXP_BLOCKING)) != SXP_SUCCESS) {
    printf("Error while creating socket! %i\n", status);
    return 1;
  }

  printf("Socket: %i\n", socket);

  if ((status = sxp_destroy(&socket)) != SXP_SUCCESS) {
    printf("Error while destroying socket! %i\n", status);
    return 1;
  }

  sxp_cleanup();
  printf("sxp_cleanup\n");
  printf("Goodbye!\n");
  return 0;
}
*/
int uitest()
{
    int loop = 1;
    if (interface_init() != UI_SUCCESS)
        return -1;

    if (interface_status("SEChat", "!help - !quit") != UI_SUCCESS)
        return -1;

    while (loop) {
        const char *message;
        uiResult status;

        if (interface_tick() != UI_SUCCESS)
            return -1;
        while ((status = interface_message_recv(&message)) == UI_SUCCESS) {
            if (!handle_command(message, &loop)) {
                interface_message_send(message);
            }
        }

        if (status == UI_ERROR) {
            /*There has been an error in the ui*/
            return 1;
        }
    }

    if (interface_exit() != UI_SUCCESS)
        return -1;
    return 0;
}
