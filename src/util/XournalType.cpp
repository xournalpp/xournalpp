#include "XournalType.h"

#include "StringUtils.h"

#include <glib.h>

#include <stdlib.h>
#include <iostream>
using std::cout;
using std::endl;

#ifdef XOJ_MEMORY_CHECK_ENABLED

#define XOURNAL_TYPE_LIST_LENGTH 256

#undef XOJ_DECLARE_TYPE
#define XOJ_DECLARE_TYPE(name, id) \
	if(XOURNAL_TYPE_LIST_LENGTH < id + 1) { \
		g_error("XOURNAL_TYPE_LIST_LENGTH is to small, it should be at least %i\n", id + 1); \
		exit(-512); \
	} \
	xojTypeList[id] = #name

static bool listInited = false;
static const char* xojTypeList[XOURNAL_TYPE_LIST_LENGTH] = { 0 };

GMutex* mutex = NULL;

static void initXournalClassList()
{
	xojTypeList[0] = "Invalid_type";
}

void xoj_type_initMutex()
{
	mutex = g_mutex_new();
}

const char* xoj_type_getName(int id)
{
	if (!listInited)
	{
		initXournalClassList();
	}

	if (mutex)
	{
		g_mutex_unlock(mutex);
	}

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
	if (mutex)
	{
		g_mutex_lock(mutex);
	}
	xojInstanceList[id]++;
	if (mutex)
	{
		g_mutex_unlock(mutex);
	}
}

void xoj_memoryleak_releaseType(int id)
{
	if (mutex)
	{
		g_mutex_lock(mutex);
	}
	xojInstanceList[id]--;
	if (mutex)
	{
		g_mutex_unlock(mutex);
	}
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
			cout << bl::format("MemoryLeak: {1} objects of type: {2}") % x % xoj_type_getName(i) << endl;
		}
	}

	cout << bl::format("MemoryLeak: sum {1} objects.") % sum << endl;
}

#endif

#endif
