#include "Plugin.h"

#include <config-features.h>

#ifdef ENABLE_PLUGINS

Plugin::Plugin()
{
	XOJ_INIT_TYPE(Plugin);
}

Plugin::~Plugin()
{
	XOJ_CHECK_TYPE(Plugin);
	XOJ_RELEASE_TYPE(Plugin);
}

//void Plugin::aaaaaa()
//{
//	XOJ_CHECK_TYPE(Plugin);
//}

#endif

