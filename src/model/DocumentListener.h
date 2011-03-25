/*
 * Xournal++
 *
 * Document listener
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __DOCUMENTLISTENER_H__
#define __DOCUMENTLISTENER_H__

#include "DocumentChangeType.h"
#include "../util/XournalType.h"

class DocumentHandler;

class DocumentListener {
public:
	DocumentListener();
	virtual ~DocumentListener();

public:
	void registerListener(DocumentHandler * handler);
	void unregisterListener();

	virtual void documentChanged(DocumentChangeType type) = 0;
	virtual void pageSizeChanged(int page) = 0;
	virtual void pageChanged(int page) = 0;
	virtual void pageInserted(int page) = 0;
	virtual void pageDeleted(int page) = 0;
	virtual void pageSelected(int page) = 0;

private:
	XOJ_TYPE_ATTRIB;


	DocumentHandler * handler;
};

#endif /* __DOCUMENTLISTENER_H__ */
