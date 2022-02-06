#ifndef QI_PLUG_QOPEN_H
#define QI_PLUG_QOPEN_H

#define MAX_QI_PATH 4096

typedef struct QuickOpenEntry
{
	struct dirent* entry;
	char* parent;
	char* fullpath;
} QuickOpenEntry;

#endif

