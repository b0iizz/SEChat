#include <stdio.h>

#include "socketxp.h"

int main(void) {
  if(sxp_init() != SXP_SUCCESS) {
    printf("Error in initialization of socket abstraction layer!\n");
  }
  printf("sxp_init successful!\n");

  sxp_cleanup();
  printf("sxp_cleanup\n");
  printf("Goodbye!\n");
  return 0;
}
