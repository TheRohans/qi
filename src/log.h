#ifndef _QI_LOG
#define _QI_LOG

#include <stdio.h>

#ifdef CONFIG_DEBUG
#define LOG(M)                                                      \
    {                                                               \
    EditBuffer *dbuf = eb_find(BUF_DEBUG);                          \
    size_t nbytes = snprintf(NULL, 0, "%s",                         \
      "[QI] (:) " M "__func__" "__FILE__") + 1;                     \
    char *str = calloc(sizeof(char), nbytes);                       \
    snprintf(str, nbytes,                                           \
            "[QI] "                                                 \
            "%s (%s:%d) " M "\n",                                   \
            __func__, __FILE__, __LINE__);                          \
    eb_insert(dbuf, 0, str, nbytes);                                \
    free(str);                                                      \
    }

#else 
#define LOG(M) // M
#endif

#define QASSERT(e)      do { if (!(e)) fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #e); } while (0)

#endif // end _QI_LOG