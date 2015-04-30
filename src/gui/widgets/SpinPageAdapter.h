/*
 * Xournal++
 *
 * Handle the Page Spin Widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>

class SpinPageListener;

class SpinPageAdapter
{
public:
	SpinPageAdapter();
	virtual ~SpinPageAdapter();

public:
	GtkWidget* getWidget();

	int getPage();
	void setPage(int page);
	void setMinMaxPage(int min, int max);

	void addListener(SpinPageListener* listener);
	void removeListener(SpinPageListener* listener);

private:
	static bool pageNrSpinChangedTimerCallback(SpinPageAdapter* adapter);
	static void pageNrSpinChangedCallback(GtkSpinButton* spinbutton, SpinPageAdapter* adapter);

	void firePageChanged();

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* widget;
	int page;

	int lastTimeoutId;
	GList* listener;
};

class SpinPageListener
{
public:
	virtual void pageChanged(int page) = 0;
};
