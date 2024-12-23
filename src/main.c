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

int main(void) { return uitest(); }
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
  if (interface_init() != UI_SUCCESS) return -1;

  if (interface_status("SEChat", "!help - !quit") != UI_SUCCESS) return -1;

  while (loop) {
    const char *message;
    uiResult status;

    if (interface_tick() != UI_SUCCESS) return -1;
    while ((status = interface_message_recv(&message)) == UI_SUCCESS) {
      /*Handle Commands*/
      if (strlen(message) >= 1 && message[0] == '!') {
        ++message;
        if (!strcmp(message, "quit") || !strcmp(message, "q")) {
          loop = 0;
        } else if (!strcmp(message, "help") || !strcmp(message, "h")) {
          /*TODO: Display help message*/
          interface_message_send("!help - !h: Display this help message\n!quit - !q: Quit the application\n");
        } else if (!strcmp(message, "bottom") || !strcmp(message, "b")) {
          interface_scroll_set(0);
        } else if (!strcmp(message, "top") || !strcmp(message, "t")) {
          interface_scroll_set(32767);
        } else if (!strcmp(message, "clear") || !strcmp(message, "cl")) {
          interface_message_clear();
        }
      } /*Handle regular messages*/
      else {
        interface_message_send(message);
      }
    }

    if (status == UI_ERROR) {
      /*There has been an error in the ui*/
      return 1;
    }
  }

  if (interface_exit() != UI_SUCCESS) return -1;
  return 0;
}
