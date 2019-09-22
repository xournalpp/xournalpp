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
#include <list>

class SpinPageListener;

class SpinPageAdapter
{
public:
	SpinPageAdapter();
	virtual ~SpinPageAdapter();

public:
	GtkWidget* getWidget();

	int getPage();
	void setPage(size_t page);
	void setMinMaxPage(size_t min, size_t max);

	void addListener(SpinPageListener* listener);
	void removeListener(SpinPageListener* listener);

private:
	static bool pageNrSpinChangedTimerCallback(SpinPageAdapter* adapter);
	static void pageNrSpinChangedCallback(GtkSpinButton* spinbutton, SpinPageAdapter* adapter);

	void firePageChanged();

private:
	GtkWidget* widget;
	size_t page;

	int lastTimeoutId;
	std::list<SpinPageListener*> listener;
};

class SpinPageListener
{
public:
	virtual void pageChanged(size_t page) = 0;
	virtual ~SpinPageListener();
};
