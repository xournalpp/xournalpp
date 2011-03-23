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
// TODO: AA: type check

#ifndef __SCROLLHANDLER_H__
#define __SCROLLHANDLER_H__

class XojPage;
class Control;

#include <gtk/gtk.h>
#include "../util/XournalType.h"

class ScrollHandler {
public:
	ScrollHandler(Control * control);
	virtual ~ScrollHandler();

public:
	void goToPreviousPage();
	void goToNextPage();

	void scrollToPage(XojPage * page, double top = 0);
	void scrollToPage(int page, double top = 0);

	void scrollToSpinPange();

	void scrollToAnnotatedPage(bool next);

	bool isPageVisible(int page);

private:
	XOJ_TYPE_ATTRIB;

	Control * control;
};

#endif /* __SCROLLHANDLER_H__ */
