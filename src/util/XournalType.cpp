#include "XournalType.h"
#include <glib.h>
#include <stdlib.h>

#define XOURNAL_TYPE_LIST_LENGTH 50

#undef XOJ_DECLARE_TYPE
#define XOJ_DECLARE_TYPE(name, id) \
	if(XOURNAL_TYPE_LIST_LENGTH < id + 1) { \
		g_error("XOURNAL_TYPE_LIST_LENGTH is to small, it should be at least %i\n", id + 1); \
		exit(-512); \
	} \
	xojTypeList[id] = #name

static bool listInited = false;
static const char * xojTypeList[XOURNAL_TYPE_LIST_LENGTH] = { 0 };

static void initXournalClassList() {
	xojTypeList[0] = "Invalid_type";
#include "XournalTypeList.h"
}

const char * xoj_type_getName(int id) {
	if (!listInited) {
		initXournalClassList();
	}

	if (id < 0) {
		g_warning("Type check: id was negative (%i), object is already deleted!?", id);
		id = -id;
	}

	return xojTypeList[id];
}
