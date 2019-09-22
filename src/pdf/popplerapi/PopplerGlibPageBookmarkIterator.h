/*
 * Xournal++
 *
 * PDF Bookmark iterator interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "pdf/base/XojPdfBookmarkIterator.h"

#include <XournalType.h>
#include "PopplerGlibAction.h"

#include <poppler.h>


class PopplerGlibPageBookmarkIterator : public XojPdfBookmarkIterator
{
public:
	PopplerGlibPageBookmarkIterator(PopplerIndexIter* iter,	PopplerDocument* document);
	virtual ~PopplerGlibPageBookmarkIterator();

public:
	virtual bool next();
	virtual bool isOpen();
	virtual XojPdfBookmarkIterator* getChildIter();
	virtual XojPdfAction* getAction();

private:
	PopplerIndexIter* iter;
	PopplerDocument* document;
};

