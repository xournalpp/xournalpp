/*
 * Xournal++
 *
 * Document handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "DocumentChangeType.h"
#include "PageRef.h"

#include <XournalType.h>

#include <list>

class DocumentListener;

class DocumentHandler
{
public:
	DocumentHandler();
	virtual ~DocumentHandler();

public:
	void fireDocumentChanged(DocumentChangeType type);
	void firePageSizeChanged(size_t page);
	void firePageChanged(size_t page);
	void firePageInserted(size_t page);
	void firePageDeleted(size_t page);
	void firePageLoaded(PageRef page);
	void firePageSelected(size_t page);

private:
	void addListener(DocumentListener* l);
	void removeListener(DocumentListener* l);

private:
	std::list<DocumentListener*> listener;

	friend class DocumentListener;
};
