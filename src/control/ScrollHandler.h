/*
 * Xournal++
 *
 * Scroll handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/widgets/SpinPageAdapter.h"
#include "model/PageRef.h"

#include <XournalType.h>

#include <gtk/gtk.h>

class XojPage;
class Control;

class ScrollHandler : public SpinPageListener
{
public:
	ScrollHandler(Control* control);
	virtual ~ScrollHandler();

public:
	void goToPreviousPage();
	void goToNextPage();

	void goToLastPage();
	void goToFirstPage();

	void scrollToPage(const PageRef& page, double top = 0);
	void scrollToPage(size_t page, double top = 0);

	void scrollToAnnotatedPage(bool next);

	bool isPageVisible(size_t page, int* visibleHeight = nullptr);

public:
	virtual void pageChanged(size_t page);

private:
	void scrollToSpinPage();

private:
	Control* control = nullptr;
};
