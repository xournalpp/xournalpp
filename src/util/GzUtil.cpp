#include "GzUtil.h"

gzFile GzUtil::openPath(Path path, string flags)
{
#ifdef WIN32
	wchar_t* wfilename = g_utf8_to_utf16(path.c_str(), -1, NULL, NULL, NULL);
	gzFile fp = gzopen_w(wfilename, flags.c_str());
	g_free(wfilename);

	return fp;
#else
	return gzopen(path.c_str(), flags.c_str());
#endif
}
