#ifndef INTERFACE_H_
#define INTERFACE_H_

enum uiresults {
  UI_SUCCESS = 0,
  UI_TRY_AGAIN = 1,
  UI_ERROR = -1
};

typedef int uiResult;

uiResult interface_init();
uiResult interface_exit();

uiResult interface_tick();

uiResult interface_message_recv(const char **message);
uiResult interface_message_send(const char *message);
uiResult interface_message_clear();
uiResult interface_scroll_set(int scroll);
uiResult interface_scroll_relative(int messages, int pages);
uiResult interface_cursor_set(int cursor);
uiResult interface_status(const char *title, const char *info);

#endif /*INTERFACE_H_*/
