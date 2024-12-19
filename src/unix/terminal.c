#include "terminalxp.h"

txpResult txp_init() {
}
txpResult txp_exit() {
}

txpResult txp_state_get(term_state_t *save) {
  return tcgetattr(STDIN_FILENO, save) < 0 ? TXP_ERROR : TXP_SUCCESS;
}
txpResult txp_state_set(term_state_t *save) {
  return tcsetattr(STDIN_FILENO, TCSAFLUSH,  save) < 0 ? TXP_ERROR : TXP_SUCCESS;
}

static term_state_t reset_terminal_state = {0};

txpResult txp_start() {
  term_state_t to_modify, read;
  txpResult result;
  if ((result = txp_state_get(&read)) != TXP_SUCCESS) return result;
  reset_terminal_state = read;
  to_modify = read;

  to_modify.c_iflag &= ~IGNCR;
  to_modify.c_oflag |= OPOST | ONLCR;
  to_modify.c_lflag &= ~ECHO & ~ICANON;
  to_modify.c_lflag |= ISIG;
  to_modify.c_cflag |= 0;
  to_modify.c_cc[VMIN] = 0;
  to_modify.c_cc[VTIME] = 0;

  if ((result = txp_state_set(&to_modify)) != TXP_SUCCESS) return result;
  if ((result = txp_state_get(&read)) != TXP_SUCCESS) return result;
  if (memcmp(&to_modify, &read, sizeof(term_state_t))) return TXP_ERROR;

  return TXP_SUCCESS;
}


txpResult txp_stop() {
  txpResult result = txp_state_set(&reset_terminal_state);
  if (result != TXP_SUCCESS) return result;
  return TXP_SUCCESS;
}

txpResult txp_send(const char *str) {
  if(write(STDOUT_FILENO, str, strlen(str)) < 0) return TXP_ERROR;
  return TXP_SUCCESS;
}

txpResult txp_recv(char *buffer, size_t *nread, size_t blen) {
  ssize_t status;
  if((status = read(STDIN_FILENO, buffer, blen)) < 0) return TXP_ERROR;
  *nread = status;
  if (status == 0) return TXP_TRY_AGAIN;
  return TXP_SUCCESS;
}

txpResult txp_flush() {
  if (tcflush(STDIN_FILENO, TCIFLUSH) < 0) return TXP_ERROR;
  return TXP_SUCCESS;
}

