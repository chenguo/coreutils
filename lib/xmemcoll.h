#include <stddef.h>
int xmemcoll (char *, size_t, char *, size_t);
int xmemcoll_nul (char *, size_t, char *, size_t);
static inline int collate_err (char *, size_t, char *, size_t, int);
