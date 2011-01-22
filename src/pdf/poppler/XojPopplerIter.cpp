#include "XojPopplerIter.h"
#include "XojPopplerDocument.h"

#include <poppler/Outline.h>
#include <poppler/GlobalParams.h>
#include <poppler/UnicodeMap.h>

XojPopplerIter::XojPopplerIter(XojPopplerDocument doc, GooList *items) {
	this->doc = doc;
	this->items = items;
	this->index = 0;
}

XojPopplerIter::~XojPopplerIter() {
}

bool XojPopplerIter::next() {
	this->index++;
	if (this->index >= this->items->getLength()) {
		return false;
	}

	return true;
}

bool XojPopplerIter::isOpen() {
	OutlineItem * item = (OutlineItem *) this->items->get(this->index);
	return item->isOpen();
}

XojPopplerIter * XojPopplerIter::getChildIter() {
	OutlineItem * item = (OutlineItem *) this->items->get(this->index);
	item->open();
	if (!(item->hasKids() && item->getKids())) {
		return NULL;
	}

	XojPopplerIter * child = new XojPopplerIter(doc, item->getKids());

	return child;
}

static gchar *
unicode_to_char(Unicode *unicode, int len) {
	static UnicodeMap *uMap = NULL;
	if (uMap == NULL) {
		GooString *enc = new GooString("UTF-8");
		uMap = globalParams->getUnicodeMap(enc);
		uMap->incRefCnt();
		delete enc;
	}

	GooString gstr;
	gchar buf[8]; /* 8 is enough for mapping an unicode char to a string */
	int i, n;

	for (i = 0; i < len; ++i) {
		n = uMap->mapUnicode(unicode[i], buf, sizeof(buf));
		gstr.append(buf, n);
	}

	return g_strdup(gstr.getCString());
}

XojPopplerAction * XojPopplerIter::getAction() {
	OutlineItem * item = (OutlineItem *) this->items->get(this->index);
	if (item == NULL) {
		return NULL;
	}
	LinkAction * linkAction = item->getAction();

	char * title = unicode_to_char(item->getTitle(), item->getTitleLength());

	return new XojPopplerAction(this->doc, linkAction, title);
}

