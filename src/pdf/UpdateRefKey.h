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

#ifndef __UPDATEREFKEY_H__
#define __UPDATEREFKEY_H__

#include "poppler/XojPopplerDocument.h"

class UpdateRefKey {
public:
	UpdateRefKey(Ref ref, XojPopplerDocument doc) {
		this->ref = ref;
		this->doc = doc;
	}

public:
	static guint hashFunction(UpdateRefKey * key);
	static bool equalFunction(UpdateRefKey * a, UpdateRefKey * b);

	static void destroyDelete(UpdateRefKey * data);

public:

	Ref ref;
	XojPopplerDocument doc;
};

#endif /* __UPDATEREFKEY_H__ */
