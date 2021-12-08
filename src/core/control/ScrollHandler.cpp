#include "ScrollHandler.h"

#include "gui/Layout.h"
#include "gui/XournalView.h"
#include "gui/widgets/SpinPageAdapter.h"

#include "Control.h"

ScrollHandler::ScrollHandler(Control* control): control(control) {}

ScrollHandler::~ScrollHandler() = default;

void ScrollHandler::goToPreviousPage() {
    if (this->control->getWindow()) {
        scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() - 1);
    }
}

void ScrollHandler::goToNextPage() {
    if (this->control->getWindow()) {
        scrollToPage(this->control->getWindow()->getXournal()->getCurrentPage() + 1);
    }
}

void ScrollHandler::goToLastPage() {
    if (this->control->getWindow()) {
        scrollToPage((*this->control->getDocument())->getPageCount() - 1);
    }
}

void ScrollHandler::goToFirstPage() {
    if (this->control->getWindow()) {
        scrollToPage(0);
    }
}

void ScrollHandler::scrollToPage(const PageRef& page, double top) {
    auto p = (*this->control->getDocument())->indexOf(page);

    if (p != npos) {
        scrollToPage(p, top);
    }
}

void ScrollHandler::scrollToPage(size_t page, double top) {
    MainWindow* win = this->control->getWindow();
    if (win == nullptr) {
        g_error("Window is nullptr!");
        return;
    }

    win->getXournal()->scrollTo(page, top);
}

void ScrollHandler::scrollToSpinPage() {
    if (!this->control->getWindow()) {
        return;
    }
    SpinPageAdapter* spinPageNo = this->control->getWindow()->getSpinPageNo();
    int page = spinPageNo->getPage();
    if (page == 0) {
        return;
    }
    scrollToPage(page - 1);
}

void ScrollHandler::scrollToAnnotatedPage(bool next) {
    if (!this->control->getWindow()) {
        return;
    }

    int step = next ? 1 : -1;

    Monitor<Document>::LockedMonitor lockedDoc = this->control->getDocument()->lock();

    for (size_t i = this->control->getCurrentPageNo() + step; i != npos && i < lockedDoc->getPageCount();
         i = ((i == 0 && step == -1) ? npos : i + step)) {
        if (lockedDoc->getPage(i)->isAnnotated()) {
            scrollToPage(i);
            break;
        }
    }
}

auto ScrollHandler::isPageVisible(size_t page, int* visibleHeight) -> bool {
    if (!this->control->getWindow()) {
        if (visibleHeight) {
            *visibleHeight = 0;
        }
        return false;
    }

    return this->control->getWindow()->getXournal()->isPageVisible(page, visibleHeight);
}

void ScrollHandler::pageChanged(size_t page) { scrollToSpinPage(); }
