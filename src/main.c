#include <stdio.h>

#include "socketxp.h"

int main(void) {
  sxp_t socket;
  sxpResult status;
  if((status = sxp_init()) != SXP_SUCCESS) {
    printf("Error in initialization of socket abstraction layer! %i\n", status);
    return 1;
  }
  printf("sxp_init successful!\n");

  if((status = sxp_create(&socket, AF_INET, SOCK_STREAM, 0, SXP_BLOCKING)) != SXP_SUCCESS) {
    printf("Error while creating socket! %i\n", status);
    return 1;
  }

  printf("Socket: %i\n", socket);

  if((status = sxp_destroy(&socket)) != SXP_SUCCESS) {
    printf("Error while destroying socket! %i\n", status);
    return 1;
  }

  sxp_cleanup();
  printf("sxp_cleanup\n");
  printf("Goodbye!\n");
  return 0;
}
