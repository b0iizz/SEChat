#include "util.h"

static int split_recurse(char ***result, size_t count, char *src);

int util_strcpy(char **dst, const char *src, int on_success, int on_failure)
{
  if (!src || !dst) return on_failure;
  *dst = malloc(strlen(src) + 1);
  if (!*dst) return on_failure;
  if (!memcpy(*dst, src, strlen(src) + 1)) {
    free(*dst);
    return on_failure;
  }
  return on_success;
}

int util_startswith(const char *str, const char *prefix)
{
  while (*prefix != '\0') {
    if (*(str++) != *(prefix++)) return 0;
  }
  return 1;
}

int util_split(char ***split, char *src)
{
  int result;
  *split = NULL;
  result = split_recurse(split, 0, src);
  if (!result) {
    free(*split);
    *split = NULL;
  }
  return result;
}

static int split_recurse(char ***result, size_t count, char *src)
{
  size_t length;
  char *tmp;

  while (*src == ' ') src++;

  length = strlen(src);

  *result = realloc(*result, (count + 1) * sizeof(**result));
  if (!result) return 0;

  if (length == 0) {
    (*result)[count] = NULL;
    return 1;
  }

  tmp = strchr(src, ' ');
  if (tmp) {
    *tmp = '\0';
    length = strlen(src);
  }

  (*result)[count] = src;
  return split_recurse(result, count + 1, src + length + (tmp != NULL));
}
