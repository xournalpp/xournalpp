/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
// TODO: AA: type check

#ifndef __UPDATEREF_H__
#define __UPDATEREF_H__

#include "poppler/XojPopplerDocument.h"

class UpdateRef {
public:
	UpdateRef(int objectId, XojPopplerDocument doc);
	static void destroyDelete(UpdateRef * data);

public:
	int objectId;
	bool wroteOut;

	Object object;
	XojPopplerDocument doc;
};

#endif /* __UPDATEREF_H__ */
