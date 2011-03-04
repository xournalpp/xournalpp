/*
 * Xournal++
 *
 * Xournal main entry, commandline parser
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XOURNALMAIN_H__
#define __XOURNALMAIN_H__

#include <config.h>

class GladeSearchpath;

class XournalMain {
public:
	XournalMain();
	virtual ~XournalMain();

public:
	int run(int argc, char *argv[]);

private:
#ifdef ENABLE_NLS
	void initLocalisation();
#endif

	int exportPdf(const char * input, const char * output);
	GladeSearchpath * initPath(const char * argv0);

};

#endif /* __XOURNALMAIN_H__ */
