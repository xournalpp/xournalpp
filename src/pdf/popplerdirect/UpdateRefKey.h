/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include "poppler/XojPopplerDocument.h"

class UpdateRefKey
{
public:
	UpdateRefKey(Ref ref, XojPopplerDocument doc)
	{
		this->ref = ref;
		this->doc = doc;
	}

public:
	static guint hashFunction(UpdateRefKey* key);
	static bool equalFunction(UpdateRefKey* a, UpdateRefKey* b);

	static void destroyDelete(UpdateRefKey* data);

public:

	Ref ref;
	XojPopplerDocument doc;
};
