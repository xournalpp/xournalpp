/*
 * Xournal++
 *
 * Custom Poppler access library
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XOJ_POPPLERDOCUMENT_H__
#define __XOJ_POPPLERDOCUMENT_H__
// TODO: AA: type check

#include "XojPopplerPage.h"
#include "../../util/String.h"

class _IntPopplerDocument;
class XojPopplerIter;

class XojPopplerDocument {
public:
	XojPopplerDocument();
	XojPopplerDocument(const XojPopplerDocument & doc);
	virtual ~XojPopplerDocument();

public:
	void operator=(XojPopplerDocument & doc);
	bool operator==(XojPopplerDocument & doc);

	XojPopplerIter * getContentsIter();

	XojPopplerPage * getPage(int page);

	bool isLoaded();

	int getPageCount();

	void load(char *data, int length);
	bool load(const char *uri, const char *password, GError **error);

	PDFDoc * getDoc();

	gsize getId();

	bool save(String filename, GError ** error);

private:
	_IntPopplerDocument * data;
};

#endif /* __XOJ_POPPLERDOCUMENT_H__ */
