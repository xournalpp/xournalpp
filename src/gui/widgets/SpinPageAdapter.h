/*
 * Xournal++
 *
 * Handle the Page Spin Widget
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SPINPAGEADAPTER_H__
#define __SPINPAGEADAPTER_H__

#include <XournalType.h>
#include <gtk/gtk.h>

class SpinPageListener;

class SpinPageAdapter {
public:
	SpinPageAdapter();
	virtual ~SpinPageAdapter();

public:
	GtkWidget * getWidget();

	int getPage();
	void setPage(int page);
	void setMinMaxPage(int min, int max);

	void addListener(SpinPageListener * listener);
	void removeListener(SpinPageListener * listener);

private:
	static bool pageNrSpinChangedTimerCallback(SpinPageAdapter * adapter);
	static void pageNrSpinChangedCallback(GtkSpinButton * spinbutton, SpinPageAdapter * adapter);

	void firePageChanged();

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget * widget;
	int page;

	int lastTimeoutId;
	GList * listener;
};

class SpinPageListener {
public:
	virtual void pageChanged(int page) = 0;
};

#endif /* __SPINPAGEADAPTER_H__ */
