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

};

#endif /* __XOURNALMAIN_H__ */
