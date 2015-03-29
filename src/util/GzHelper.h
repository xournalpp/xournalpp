/*
 * Xournal++
 *
 * Helper functions for ZLib GZip Compression
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __GZHELPER_H__
#define __GZHELPER_H__

#include <StringUtils.h>

//Rewriting with boost:iostreams would be pointless
class GzHelper
{
private:
	GzHelper();
	virtual ~GzHelper();

public:
	static string gzcompress(const string& str, int level = -1);
	static string gzuncompress(const string& str);
};

#endif /* __GZHELPER_H__ */
