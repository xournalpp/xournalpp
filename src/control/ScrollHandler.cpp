#include "ScrollHandler.h"
#include "../model/Page.h"
#include "Control.h"
#include "../gui/XournalView.h"

ScrollHandler::ScrollHandler(Control * control) {
	XOJ_INIT_TYPE(ScrollHandler);

	this->control = control;
}

ScrollHandler::~ScrollHandler() {
	XOJ_RELEASE_TYPE(ScrollHandler);
}

void ScrollHandler::goToPreviousPage() {
	XOJ_CHECK_TYPE(ScrollHandler);

	if (this->control->getWindow()) {
		scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() - 1);
	}
}

void ScrollHandler::goToNextPage() {
	XOJ_CHECK_TYPE(ScrollHandler);

	if (this->control->getWindow()) {
		scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() + 1);
	}
}

void ScrollHandler::scrollToPage(XojPage * page, double top) {
	XOJ_CHECK_TYPE(ScrollHandler);

	Document * doc = this->control->getDocument();

	doc->lock();
	int p = doc->indexOf(page);
	doc->unlock();

	if (p != -1) {
		scrollToPage(p, top);
	}
}

void ScrollHandler::scrollToPage(int page, double top) {
	XOJ_CHECK_TYPE(ScrollHandler);

	if (this->control->getWindow()) {
		this->control->getWindow()->getXournal()->scrollTo(page, top);
	}
}

void ScrollHandler::scrollToSpinPange() {
	XOJ_CHECK_TYPE(ScrollHandler);

	if (!this->control->getWindow()) {
		return;
	}
	GtkWidget * spinPageNo = this->control->getWindow()->getSpinPageNo();
	int page = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinPageNo));
	if (page == 0) {
		return;
	}
	scrollToPage(page - 1);
}

void ScrollHandler::scrollToAnnotatedPage(bool next) {
	XOJ_CHECK_TYPE(ScrollHandler);

	if (!this->control->getWindow()) {
		return;
	}

	int step;
	if (next) {
		step = 1;
	} else {
		step = -1;
	}

	Document * doc = this->control->getDocument();
	doc->lock();

	for (int i = this->control->getCurrentPageNo() + step; i >= 0 && i < doc->getPageCount(); i += step) {
		if (doc->getPage(i)->isAnnotated()) {
			scrollToPage(i);
			break;
		}
	}

	doc->unlock();
}

bool ScrollHandler::isPageVisible(int page) {
	XOJ_CHECK_TYPE(ScrollHandler);

	if (!this->control->getWindow()) {
		return false;
	}

	return this->control->getWindow()->getXournal()->isPageVisible(page);
}

