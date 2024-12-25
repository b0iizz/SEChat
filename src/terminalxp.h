#ifndef TERMINALXP_H_
#define TERMINALXP_H_

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <conio.h>

typedef struct {
    DWORD input_mode;
    DWORD output_mode;
} term_state_t;

#else

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

typedef struct termios term_state_t;

#endif /*WIN32*/

typedef int txpResult;

enum txpresults { TXP_SUCCESS = 0, TXP_TRY_AGAIN = 1, TXP_ERROR = -1 };

txpResult txp_init();
txpResult txp_exit();

txpResult txp_state_get(term_state_t *save);
txpResult txp_state_set(term_state_t *save);

txpResult txp_start();
txpResult txp_stop();

txpResult txp_send(const char *str);
txpResult txp_recv(char *buffer, size_t *nread, size_t blen);
txpResult txp_flush();

#endif /*TERMINALXP_H_*/
