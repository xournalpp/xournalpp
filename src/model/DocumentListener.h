/*
 * Xournal++
 *
 * Document listener
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "DocumentChangeType.h"
#include <XournalType.h>

class DocumentHandler;

class DocumentListener
{
public:
	DocumentListener();
	virtual ~DocumentListener();

public:
	void registerListener(DocumentHandler* handler);
	void unregisterListener();

	virtual void documentChanged(DocumentChangeType type) = 0;
	virtual void pageSizeChanged(size_t page) = 0;
	virtual void pageChanged(size_t page) = 0;
	virtual void pageInserted(size_t page) = 0;
	virtual void pageDeleted(size_t page) = 0;
	virtual void pageSelected(size_t page) = 0;

private:
	XOJ_TYPE_ATTRIB;

	DocumentHandler* handler;
};
