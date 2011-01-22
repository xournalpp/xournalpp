/*
 * Xournal++
 *
 * Helper functions for ZLib GZip Compression
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */


#ifndef __GZHELPER_H__
#define __GZHELPER_H__

#include <glib.h>

class GzHelper {
private:
	GzHelper();
	virtual ~GzHelper();

public:
	static GString * gzcompress(GString* str, int level = -1);

};

#endif /* __GZHELPER_H__ */
