#ifndef _QI_LOG
#define _QI_LOG

#include <stdio.h>

#ifdef CONFIG_DEBUG
#define LOG(M, ...)                                        \
    do {                                                   \
       fprintf(stderr,                                     \
            "[QI] %s (%s:%d) " M "\n",                     \
            __func__, __FILE__, __LINE__, __VA_ARGS__);    \
    } while(0);                                            

#else 
#define LOG(M, ...) // M
#endif

#define QASSERT(e)                                                    \
    do {                                                              \
        if (!(e))                                                     \
           fprintf(stderr,                                            \
            "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #e); \
    } while (0);

#endif // end _QI_LOG