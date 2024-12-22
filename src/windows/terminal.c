#include "terminalxp.h"

static term_state_t terminal_reset_state = {0};
static HANDLE terminal_con_in;
static HANDLE terminal_con_out;

static txpResult map_key(char *buf, size_t *nread, size_t blen, int keycode);

txpResult txp_init()
{
  if ((terminal_con_in = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
    return TXP_ERROR;
  }
  if ((terminal_con_out = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
    return TXP_ERROR;
  }
  return TXP_SUCCESS;
}
txpResult txp_exit()
{
  if (!CloseHandle(terminal_con_in)) return TXP_ERROR;
  if (!CloseHandle(terminal_con_out)) return TXP_ERROR;

  return TXP_SUCCESS;
}

txpResult txp_state_get(term_state_t *save)
{
  if (!save) return TXP_ERROR;
  if (!GetConsoleMode(terminal_con_in, &(save->input_mode))) return TXP_ERROR;
  if (!GetConsoleMode(terminal_con_out, &(save->output_mode))) return TXP_ERROR;
  return TXP_SUCCESS;
}

txpResult txp_state_set(term_state_t *save)
{
  if (!save) return TXP_ERROR;
  if (!SetConsoleMode(terminal_con_in, save->input_mode)) return TXP_ERROR;
  if (!SetConsoleMode(terminal_con_out, save->output_mode)) return TXP_ERROR;

  return TXP_SUCCESS;
}

txpResult txp_start()
{
  term_state_t to_modify, read;
  txpResult result;

  if ((result = txp_state_get(&read)) != TXP_SUCCESS) return result;
  terminal_reset_state = read;
  to_modify = read;

  to_modify.input_mode |= ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_PROCESSED_INPUT
                          | ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT;
  to_modify.input_mode &=
      ~ENABLE_ECHO_INPUT & ~ENABLE_INSERT_MODE & ~ENABLE_LINE_INPUT & ~ENABLE_QUICK_EDIT_MODE;
  to_modify.output_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT
                           | ENABLE_WRAP_AT_EOL_OUTPUT | DISABLE_NEWLINE_AUTO_RETURN;

  if ((result = txp_state_set(&to_modify)) != TXP_SUCCESS) return result;
  if ((result = txp_state_get(&read)) != TXP_SUCCESS) return result;
  if (memcmp(&to_modify, &read, sizeof(term_state_t))) return TXP_ERROR;

  return TXP_SUCCESS;
}

txpResult txp_stop()
{
  txpResult result = txp_state_set(&terminal_reset_state);
  if (result != TXP_SUCCESS) return result;
  return TXP_SUCCESS;
}


txpResult txp_send(const char *str)
{
  printf("%s", str);
  return TXP_SUCCESS;
}

txpResult txp_flush()
{
  if (!FlushConsoleInputBuffer(terminal_con_in)) return TXP_ERROR;
  return TXP_SUCCESS;
}

static txpResult map_key(char *buf, size_t *nread, size_t blen, int keycode)
{
  const char *escseq;
  switch (keycode) {
    case VK_UP: escseq = "\033[A"; break;
    case VK_DOWN: escseq = "\033[B"; break;
    case VK_RIGHT: escseq = "\033[C"; break;
    case VK_LEFT: escseq = "\033[D"; break;
    default: escseq = ""; break;
  }
  if (strlen(escseq) + *nread >= blen) return TXP_TRY_AGAIN;
  while (*escseq != '\0') buf[(*nread)++] = *(escseq++);
  return TXP_SUCCESS;
}

txpResult txp_recv(char *buffer, size_t *nread, size_t blen)
{
  DWORD can_read, did_read;
  INPUT_RECORD input;
  *nread = 0;

  /*Maybe use WaitForSingleObject here aswell...*/
  if (!GetNumberOfConsoleInputEvents(terminal_con_in, &can_read)) return TXP_ERROR;
  if (can_read <= 0) return TXP_TRY_AGAIN;
  while (can_read > 0 && *nread < blen) {
    if (!PeekConsoleInput(terminal_con_in, &input, 1, &did_read) || did_read <= 0) return TXP_ERROR;
    switch (input.EventType) {
      int code;
      char ascii;
      txpResult mapResult;
      char buf[11];
      case KEY_EVENT:
        code = input.Event.KeyEvent.wVirtualKeyCode;
        ascii = input.Event.KeyEvent.uChar.AsciiChar;

        if (input.Event.KeyEvent.bKeyDown) {
          if (ascii == 0) {
            if ((mapResult = map_key(buffer, nread, blen, code)) != TXP_SUCCESS)
              return mapResult == TXP_TRY_AGAIN ? TXP_SUCCESS : TXP_ERROR;
          } else {
            buffer[(*nread)++] = ascii;
          }
        }
        if (!ReadConsoleInput(terminal_con_in, &input, 1, &did_read) || did_read <= 0)
          return TXP_ERROR;
        can_read--;
        break;
      case WINDOW_BUFFER_SIZE_EVENT:
         sprintf(buf, "\033[%i;%iR", input.Event.WindowBufferSizeEvent.dwSize.X, input.Event.WindowBufferSizeEvent.dwSize.Y);
         if (*nread + strlen(buf) >= blen) return TXP_SUCCESS; /*Prevent Event from being consumed*/
         memcpy(buf + *nread, buf, strlen(buf));
         *nread += strlen(buf);
        if (!ReadConsoleInput(terminal_con_in, &input, 1, &did_read) || did_read <= 0)
          return TXP_ERROR;
        can_read--;
      default:
        if (!ReadConsoleInput(terminal_con_in, &input, 1, &did_read) || did_read <= 0)
          return TXP_ERROR;
        can_read--;
        break;
    }
  }
  return TXP_SUCCESS;
}
