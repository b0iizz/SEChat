#include "interface.h"
#include "terminalxp.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INPUT_LENGTH 128
#define COMMAND_BUF_LENGTH 8192
#define MAX_LINES 80

#define INPUT_LINE terminal_height
#define STATUS_LINE 1

static int terminal_height = 24;
static int terminal_width = 80;
static time_t last_since_checked_size = 0;

static char *messages[MAX_LINES] = { 0 };
static int messages_count = 0;
static int messages_scroll = 0;
static int messages_dirty = 1;
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define MESSAGES_DIRTY 1
#else
#define MESSAGES_DIRTY messages_dirty
#endif

static char inputs[INPUT_LENGTH] = { 0 };
static int inputs_cursor = 0;
static int inputs_dirty = 1;
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define INPUTS_DIRTY 1
#else
#define INPUTS_DIRTY inputs_dirty
#endif

static char actions[INPUT_LENGTH] = { 0 };
static int actions_dirty = 1;

static char *status = NULL;
static char *status_info = NULL;
static int status_dirty = 1;
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define STATUS_DIRTY 1
#else
#define STATUS_DIRTY status_dirty
#endif

static uiResult bounds_tick();

static size_t input_consume_csi(char *buf, size_t len);
static uiResult input_handle_csi(char *buf, size_t csilen);
static uiResult input_tick();

static char command_buffer[COMMAND_BUF_LENGTH];

static uiResult render_start();
static uiResult render_stop();

static uiResult render_status();
static uiResult render_messages();
static uiResult render_input();

uiResult interface_init()
{
    if (txp_init() != TXP_SUCCESS)
        return UI_ERROR;
    if (txp_start() != TXP_SUCCESS)
        return UI_ERROR;
    if (render_start() != UI_SUCCESS)
        return UI_ERROR;
    if (bounds_tick() != UI_SUCCESS)
        return UI_ERROR;
    return UI_SUCCESS;
}

uiResult interface_exit()
{
    int i;
    for (i = 0; i < MAX_LINES && i < messages_count; i++) {
        free(messages[i]);
    }
    if (status) {
        free(status);
    }

    if (render_stop() != UI_SUCCESS)
        return UI_ERROR;
    if (txp_stop() != TXP_SUCCESS)
        return UI_ERROR;
    if (txp_exit() != TXP_SUCCESS)
        return UI_ERROR;
    return UI_SUCCESS;
}

uiResult interface_message_recv(const char **message)
{
    if (!actions_dirty)
        return UI_TRY_AGAIN;

    actions_dirty = 0;
    *message = actions;

    return UI_SUCCESS;
}

uiResult interface_message_send(const char *message)
{
    char *tmp_tok;
    char *tmp_copy = malloc(strlen(message) + 1);
    if (!tmp_copy)
        return UI_ERROR;
    memcpy(tmp_copy, message, strlen(message) + 1);

    tmp_tok = strtok(tmp_copy, "\n");
    if (!strchr(tmp_copy, '\n'))
        tmp_tok = tmp_copy;
    while (tmp_tok) {
        char *local_copy;
        int i;

        if (messages[messages_count % MAX_LINES]) {
            free(messages[messages_count % MAX_LINES]);
            messages[messages_count % MAX_LINES] = NULL;
        }
        messages_dirty = 1;

        local_copy = malloc(strlen(tmp_tok) + 1);
        if (!local_copy)
            goto error;
        memcpy(local_copy, tmp_tok, strlen(tmp_tok) + 1);

        for (i = 0; local_copy[i] != '\0'; i++)
            local_copy[i] = isprint(local_copy[i]) ? local_copy[i] : '#';

        messages[messages_count++ % MAX_LINES] = local_copy;

        tmp_tok = strtok(NULL, "\n");
    }
    free(tmp_copy);
    return UI_SUCCESS;
error:
    free(tmp_copy);
    return UI_ERROR;
}

uiResult interface_message_clear()
{
    int i;
    for (i = 0; i < MAX_LINES && i < messages_count; i++) {
        free(messages[i]);
        messages[i] = NULL;
    }
    messages_count = 0;
    messages_scroll = 0;
    messages_dirty = 1;
    return UI_SUCCESS;
}

uiResult interface_scroll_set(int scroll)
{
    messages_scroll = scroll;
    messages_scroll = messages_scroll <= MAX_LINES - 1 ? messages_scroll :
                                                         MAX_LINES - 1;
    messages_scroll = messages_scroll <= messages_count - 2 ?
                          messages_scroll :
                          messages_count - 2;
    messages_scroll = messages_scroll >= 0 ? messages_scroll : 0;
    messages_dirty = 1;

    return UI_SUCCESS;
}

uiResult interface_scroll_relative(int messages, int pages)
{
    return interface_scroll_set(messages_scroll + messages +
                                pages * (INPUT_LINE - STATUS_LINE - 1));
}

uiResult interface_cursor_set(int cursor)
{
    inputs_cursor = cursor;
    inputs_cursor = inputs_cursor <= (int)strlen(inputs) ? inputs_cursor :
                                                           (int)strlen(inputs);
    inputs_cursor = inputs_cursor >= 0 ? inputs_cursor : 0;
    inputs_dirty = 1;
    return UI_SUCCESS;
}

uiResult interface_status(const char *title, const char *info)
{
    char *local_copy;

    if (status) {
        free(status);
        status = NULL;
    }
    if (status_info) {
        free(status_info);
        status_info = NULL;
    }
    status_dirty = 1;

    local_copy = malloc(strlen(title) + 1);
    if (!local_copy)
        return UI_ERROR;
    memcpy(local_copy, title, strlen(title) + 1);

    status = local_copy;

    local_copy = malloc(strlen(info) + 1);
    if (!local_copy)
        return UI_ERROR;
    memcpy(local_copy, info, strlen(info) + 1);

    status_info = local_copy;

    return UI_SUCCESS;
}

uiResult interface_tick()
{
    if (input_tick() != UI_SUCCESS)
        return UI_ERROR;
    if (STATUS_DIRTY && (render_status() != UI_SUCCESS))
        return UI_ERROR;
    if (MESSAGES_DIRTY && (render_messages() != UI_SUCCESS))
        return UI_ERROR;
    if (INPUTS_DIRTY && (render_input() != UI_SUCCESS))
        return UI_ERROR;
    if (bounds_tick() != UI_SUCCESS)
        return UI_ERROR;
    return UI_SUCCESS;
}

static uiResult bounds_tick()
{
    time_t current_time = time(NULL);
    if (current_time - last_since_checked_size < 1)
        return UI_SUCCESS;
    last_since_checked_size = current_time;

    sprintf(command_buffer, "\033[s\033[999;999H\033[6n\033[u");
    if (txp_send(command_buffer) != TXP_SUCCESS)
        return UI_ERROR;
    return UI_SUCCESS;
}

static size_t input_consume_csi(char *buf, size_t len)
{
    size_t result = 1;
    if (len <= 1 || buf[0] != '[')
        return result;
    for (; result < len; result++)
        if (!strchr("0123456789:;<=>?", buf[result]))
            break;
    for (; result < len; result++)
        if (!strchr("!\"#$%&\'()*+,-./", buf[result])) {
            break;
        }
    if (result >= len || !(buf[result] >= '@' && buf[result] <= '~'))
        return 1;
    return result + 1;
}

static uiResult input_handle_csi(char *buf, size_t csilen)
{
    int scanidx = 0;
    int read = 0;
    unsigned int amount;
    unsigned int x;
    unsigned int y;
    char command = buf[csilen - 1];
    /*char test[128] = {0};
  sprintf(test, "%.*s : %li\n", csilen, buf, csilen);
  interface_message_send(test);*/
    switch (command) {
    case 'A':
        if (csilen == 1) {
            amount = 1;
        } else if (sscanf(buf, "%uA", &amount) != 1)
            goto csi_end;
        interface_scroll_set(messages_scroll + amount);
        break;
    case 'B':
        if (csilen == 1) {
            amount = 1;
        } else if (sscanf(buf, "%uB", &amount) != 1)
            goto csi_end;
        interface_scroll_set(messages_scroll - amount);
        break;
    case 'C':
        if (csilen == 1) {
            amount = 1;
        } else if (sscanf(buf, "%uC", &amount) != 1)
            goto csi_end;
        interface_cursor_set(inputs_cursor + amount);
        break;
    case 'D':
        if (csilen == 1) {
            amount = 1;
        } else if (sscanf(buf, "%uD", &amount) != 1)
            goto csi_end;
        interface_cursor_set(inputs_cursor - amount);
        break;
    case 'R':
        if (sscanf(buf + scanidx, "%u%n", &y, &read) == 1) {
            scanidx += read;
        } else {
            y = 1;
        }
        if (buf[scanidx++] != ';')
            goto csi_end;
        if (sscanf(buf + scanidx, "%u%n", &x, &read) == 1) {
            scanidx += read;
        } else {
            x = 1;
        }
        if (buf[scanidx++] != 'R')
            goto csi_end;
        terminal_width = x;
        terminal_height = y;

        status_dirty = 1;
        messages_dirty = 1;
        inputs_dirty = 1;
        break;
    default:
        break;
    }
csi_end:
    return UI_SUCCESS;
}

static uiResult input_tick()
{
    char readbuf[32];
    size_t nread;
    size_t i;
    size_t j;
    txpResult status;

    if ((status =
             txp_recv(readbuf, &nread, sizeof(readbuf) / sizeof(*readbuf))) !=
        TXP_SUCCESS) {
        switch (status) {
        case TXP_TRY_AGAIN:
            return UI_SUCCESS;
        case TXP_ERROR:
        default:
            return UI_ERROR;
        }
    }
    for (i = 0; i < nread;) {
        switch (readbuf[i]) {
        case '\r':
        case '\n':
            memcpy(actions, inputs, strlen(inputs) + 1);
            actions_dirty = 1;

            inputs[0] = '\0';
            inputs_cursor = 0;
            inputs_dirty = 1;

            i++;
            break;
        case 127:
        case '\b':
            if (inputs_cursor > 0) {
                memmove(inputs + inputs_cursor - 1, inputs + inputs_cursor,
                        strlen(inputs + inputs_cursor) + 1);
                inputs_cursor--;

                inputs_dirty = 1;
            }

            i++;
            break;
        case '\033':
            j = input_consume_csi(readbuf + i + 1, nread - (i + 1));
            if (j <= 1 ||
                input_handle_csi(readbuf + i + 2, j - 1) != UI_SUCCESS) {
                i++;
                break;
            }
            i += j + 1;
            break;
        default:
            if (isprint(readbuf[i]) && inputs_cursor < INPUT_LENGTH - 1) {
                size_t to_copy = strlen(inputs + inputs_cursor) + 1;
                size_t available = INPUT_LENGTH - inputs_cursor - 1;
                to_copy = to_copy < available ? to_copy : available;

                memmove(inputs + inputs_cursor + 1, inputs + inputs_cursor,
                        to_copy);

                inputs[inputs_cursor++] = readbuf[i];
                inputs_dirty = 1;
            }
            i++;
            break;
        }
    }
    return UI_SUCCESS;
}

#define R_CSI "\033["

#define R_NEWSCREEN R_CSI "?1049h" R_CSI "2J"
#define R_OLDSCREEN R_CSI "?1049l"

#define R_EL R_CSI "2K"
#define R_ED R_CSI "2J"
#define R_SCP R_CSI "s"
#define R_RCP R_CSI "u"
#define R_HL R_CSI "47m" R_CSI "30m"
#define R_SGRR R_CSI "0m"

static uiResult render_start()
{
    if (txp_send(R_NEWSCREEN) != TXP_SUCCESS)
        return UI_ERROR;
    return UI_SUCCESS;
}

static uiResult render_stop()
{
    if (txp_send(R_OLDSCREEN) != TXP_SUCCESS)
        return UI_ERROR;
    return UI_SUCCESS;
}

static uiResult render_status()
{
    const char *title = status ? status : "";
    int title_pos = terminal_width / 2 - strlen(title) / 2;
    const char *info = status_info ? status_info : "";
    int info_pos = terminal_width - strlen(info) - 1;

    title_pos = title_pos + (int)strlen(title) < terminal_width ?
                    title_pos :
                    terminal_width - (int)strlen(title);
    title_pos = title_pos >= 0 ? title_pos : 0;

    info_pos = info_pos >= 0 ? info_pos : 0;

    sprintf(command_buffer,
            R_SCP R_CSI ";H" R_HL R_EL R_CSI ";%iH"
                        "%s" R_CSI ";%iH"
                        "%s" R_SGRR R_RCP,
            title_pos, title, info_pos, info);

    if (txp_send(command_buffer) != TXP_SUCCESS)
        return UI_ERROR;
    status_dirty = 0;
    return UI_SUCCESS;
}

static uiResult render_messages()
{
    int line;
    char *command = command_buffer;

    sprintf(command, R_SCP);
    command += strlen(command);

    for (line = STATUS_LINE + 1; line < INPUT_LINE; line++) {
        int line_offset = (INPUT_LINE) - (STATUS_LINE)-line;
        int message_offset = messages_count - messages_scroll - line_offset - 1;
        if (message_offset < messages_count - MAX_LINES)
            message_offset = -1;
        if (message_offset >= 0) {
            sprintf(command, R_CSI "%i;1H" R_EL "%s\n", line,
                    messages[message_offset % MAX_LINES]);
        } else {
            sprintf(command, R_CSI "%i;1H" R_EL "\n", line);
        }
        command += strlen(command);
        if (command >= command_buffer + COMMAND_BUF_LENGTH)
            return UI_ERROR;
    }

    if (command >= command_buffer + COMMAND_BUF_LENGTH)
        return UI_ERROR;

    sprintf(command, R_SGRR R_RCP);
    command += strlen(command);
    if (command >= command_buffer + COMMAND_BUF_LENGTH)
        return UI_ERROR;

    if (txp_send(command_buffer) != TXP_SUCCESS)
        return UI_ERROR;
    messages_dirty = 0;
    return UI_SUCCESS;
}

static uiResult render_input()
{
    sprintf(command_buffer, R_CSI "%i;1H" R_HL R_EL "%s" R_CSI "%i;%iH" R_SGRR,
            INPUT_LINE, inputs, INPUT_LINE, inputs_cursor + 1);

    if (txp_send(command_buffer) != TXP_SUCCESS)
        return UI_ERROR;
    inputs_dirty = 0;
    return UI_SUCCESS;
}
