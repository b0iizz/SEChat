#ifndef UTIL_H_
#define UTIL_H_

#include <stdlib.h>
#include <string.h>

int util_strcpy(char **dst, const char *src, int on_success, int on_failure);
int util_startswith(const char *str, const char *prefix);
int util_split(char ***split, char *src);

#endif /* UTIL_H_ */
