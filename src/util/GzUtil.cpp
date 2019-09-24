#include "GzUtil.h"

gzFile GzUtil::openPath(Path path, string flags)
{
#ifdef _WIN32
	wchar_t* wfilename = (wchar_t*)g_utf8_to_utf16(path.c_str(), -1, nullptr, nullptr, nullptr);
	gzFile fp = gzopen_w(wfilename, flags.c_str());
	g_free(wfilename);

	return fp;
#else
	return gzopen(path.c_str(), flags.c_str());
#endif
}
