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

class XojPage;
class Control;

#include <gtk/gtk.h>

class ScrollHandler {
public:
	ScrollHandler(Control * control);
	virtual ~ScrollHandler();

public:
	void goToPreviousPage();
	void goToNextPage();

	void scrollToPage(XojPage * page, double top = 0);
	void scrollToPage(int page, double top = 0);

	void adjustmentScroll(GtkAdjustment * adj, double scroll, int size);

	void scrollRelative(double x, double y);
	void scrollToSpinPange();

	void scrollToAnnotatedPage(bool next);

	bool isPageVisible(int page);

private:
	Control * control;
};

#endif /* __SCROLLHANDLER_H__ */
