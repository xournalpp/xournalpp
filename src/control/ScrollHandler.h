/*
 * Xournal++
 *
 * Scroll handler
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SCROLLHANDLER_H__
#define __SCROLLHANDLER_H__

class Control;

#include <gtk/gtk.h>
#include <XournalType.h>
#include "../model/PageRef.h"
#include "../gui/widgets/SpinPageAdapter.h"

class ScrollHandler: public SpinPageListener {
public:
	ScrollHandler(Control * control);
	virtual ~ScrollHandler();

public:
	void goToPreviousPage();
	void goToNextPage();

	void scrollToPage(PageRef page, double top = 0);
	void scrollToPage(int page, double top = 0);

	void scrollToSpinPange();

	void scrollToAnnotatedPage(bool next);

	bool isPageVisible(int page, int * visibleHeight = NULL);

public:
	virtual void pageChanged(int page);

private:
	XOJ_TYPE_ATTRIB;

	Control * control;
};

#endif /* __SCROLLHANDLER_H__ */
