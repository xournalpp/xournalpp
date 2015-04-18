/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "poppler/XojPopplerDocument.h"

class UpdateRef
{
public:
	UpdateRef(int objectId, XojPopplerDocument doc);
	virtual ~UpdateRef();

public:
	static void destroyDelete(UpdateRef* data);

public:
	XOJ_TYPE_ATTRIB;

	int objectId;
	bool wroteOut;

	Object object;
	XojPopplerDocument doc;
};
