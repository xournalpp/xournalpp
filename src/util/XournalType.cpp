#include "XournalType.h"

#include "StringUtils.h"
#include <config-dev.h>

#include <glib.h>

#include <stdlib.h>
#include <iostream>
using std::cout;
using std::endl;

#ifdef DEV_MEMORY_CHECKING

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
static GMutex mutex = { 0 };

static void initXournalClassList()
{
	xojTypeList[0] = "Invalid_type";

	// DO NOT REMOVE OR MOVE THIS INCLUDE!
	// It has to be included exactly here, else the macros are not working
	#include "XournalTypeList.h"
}

const char* xoj_type_getName(int id)
{
	g_mutex_lock(&mutex);
	
	if (!listInited)
	{
		initXournalClassList();
	}

	g_mutex_unlock(&mutex);

	if (id < 0)
	{
		g_warning("Type check: id was negative (%i), object is already deleted!?", id);
		id = -id;
	}

	const char* typeName = xojTypeList[id];

	if (typeName == NULL)
	{
		g_warning("Type check: name of id %i is NULL!", id);
	}

	return typeName;
}

#ifdef DEV_MEMORY_LEAK_CHECKING

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
			cout << bl::format("MemoryLeak: {1} objects of type: {2}") % x % xoj_type_getName(i) << endl;
		}
	}

	cout << bl::format("MemoryLeak: sum {1} objects.") % sum << endl;
}

#endif // DEV_MEMORY_LEAK_CHECKING

#endif // DEV_MEMORY_CHECKING
