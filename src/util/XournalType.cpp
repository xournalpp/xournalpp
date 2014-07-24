#include <XournalType.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef XOJ_MEMORY_CHECK_ENABLED

#define XOURNAL_TYPE_LIST_LENGTH 256

#undef XOJ_DECLARE_TYPE
#define XOJ_DECLARE_TYPE(name, id) \
	if(XOURNAL_TYPE_LIST_LENGTH < id + 1) { \
		g_error("XOURNAL_TYPE_LIST_LENGTH is to small, it should be at least %i\n", id + 1); \
		exit(-512); \
	} \
	xojTypeList[id] = #name

static const char* xojTypeList[XOURNAL_TYPE_LIST_LENGTH] = { 0 };

GMutex mutex;

static void initXournalClassList()
{
	xojTypeList[0] = "Invalid_type";
#include "XournalTypeList.h"
}

struct Init
{
	Init()
	{
		initXournalClassList();
		g_mutex_init(&mutex);
	}
	
} init;

const char* xoj_type_getName(int id)
{
	if (id < 0)
	{
		g_warning("Type check: id was negative (%i), object is already deleted!?", id);
		id = -id;
	}

	return xojTypeList[id];
}

#ifdef XOJ_MEMORY_LEAK_CHECK_ENABLED

static int xojInstanceList[XOURNAL_TYPE_LIST_LENGTH] = { 0 };

void xoj_memoryleak_initType(int id)
{
	g_mutex_lock(&mutex);

	xojInstanceList[id]++;

	g_mutex_unlock(&mutex);
}

void xoj_memoryleak_releaseType(int id)
{
	g_mutex_lock(&mutex);

	xojInstanceList[id]--;

	g_mutex_unlock(&mutex);

}

void xoj_momoryleak_printRemainingObjects()
{
	int sum = 0;
	for (int i = 0; i < XOURNAL_TYPE_LIST_LENGTH; i++)
	{
		int x = xojInstanceList[i];
		if (x != 0)
		{
			sum += x;
			printf("MemoryLeak: %i objects of type: %s\n", x, xoj_type_getName(i));
		}
	}

	printf("MemoryLeak: sum %i objects.\n", sum);
}

#endif

#endif
