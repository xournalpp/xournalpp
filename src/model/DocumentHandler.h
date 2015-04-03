/*
 * Xournal++
 *
 * Document handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __DOCUMENTHANDLER_H__
#define __DOCUMENTHANDLER_H__

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
	void firePageSizeChanged(int page);
	void firePageChanged(int page);
	void firePageInserted(int page);
	void firePageDeleted(int page);
	void firePageLoaded(PageRef page);
	void firePageSelected(int page);

private:
	void addListener(DocumentListener* l);
	void removeListener(DocumentListener* l);

private:
	XOJ_TYPE_ATTRIB;


	std::list<DocumentListener*> listener;

	friend class DocumentListener;
};

#endif /* __DOCUMENTHANDLER_H__ */
