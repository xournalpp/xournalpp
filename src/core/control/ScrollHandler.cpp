#include "ScrollHandler.h"

#include <memory>  // for __shared_ptr_access

#include <glib.h>  // for g_error

#include "control/NavigationHistory.h"    // for NavigationHistory
#include "control/zoom/ZoomControl.h"     // for ZoomControl
#include "gui/MainWindow.h"               // for MainWindow
#include "gui/XournalView.h"              // for XournalView
#include "gui/sidebar/Sidebar.h"          // for Sidebar
#include "gui/widgets/SpinPageAdapter.h"  // for SpinPageAdapter
#include "model/Document.h"               // for Document
#include "model/LinkDestination.h"        // for LinkDestination
#include "model/XojPage.h"                // for XojPage
#include "util/Util.h"                    // for npos

#include "Control.h"  // for Control

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
        size_t lastPage = this->control->getDocument()->getPageCount() - 1;
        if (this->control->getCurrentPageNo() != lastPage) {
            this->control->getNavigationHistory()->recordNavPoint();
        }
        scrollToPage(lastPage);
    }
}

void ScrollHandler::goToFirstPage() {
    if (this->control->getWindow()) {
        if (this->control->getCurrentPageNo() != 0) {
            this->control->getNavigationHistory()->recordNavPoint();
        }
        scrollToPage(0);
    }
}

void ScrollHandler::scrollToPage(const PageRef& page, XojPdfRectangle rect) {
    Document* doc = this->control->getDocument();

    doc->lock();
    auto p = doc->indexOf(page);
    doc->unlock();

    if (p != npos) {
        scrollToPage(p, rect);
    }
}

void ScrollHandler::scrollToPage(size_t page, XojPdfRectangle rect) {
    MainWindow* win = this->control->getWindow();
    if (win == nullptr) {
        g_error("Window is nullptr!");
        return;
    }

    win->getXournal()->scrollTo(page, rect);
}

void ScrollHandler::jumpToPage(const PageRef& page, XojPdfRectangle rect) {
    Document* doc = this->control->getDocument();

    doc->lock();
    auto p = doc->indexOf(page);
    doc->unlock();

    if (p != npos) {
        jumpToPage(p, rect);
    }
}

void ScrollHandler::jumpToPage(size_t page, XojPdfRectangle rect) {
    this->control->getNavigationHistory()->recordNavPoint();
    scrollToPage(page, rect);
}

void ScrollHandler::scrollToLinkDest(const LinkDestination& dest) {
    size_t pdfPage = dest.getPdfPage();

    if (pdfPage != npos) {
        Document* doc = control->getDocument();
        doc->lock();
        size_t page = doc->findPdfPage(pdfPage);
        doc->unlock();

        if (page == npos) {
            control->askInsertPdfPage(pdfPage);
        } else {
            if (dest.shouldChangeTop()) {
                jumpToPage(page, {dest.getLeft(), dest.getTop(), -1, -1});
            } else {
                if (control->getCurrentPageNo() != page) {
                    jumpToPage(page);
                }
            }
        }
    }
}

void ScrollHandler::scrollToAnnotatedPage(bool next) {
    if (!this->control->getWindow()) {
        return;
    }

    size_t step = next ? size_t(1) : size_t(-1);  // Don't assume blindly that size_t(-1) == npos

    Document* doc = this->control->getDocument();

    for (size_t i = this->control->getCurrentPageNo() + step; i < doc->getPageCount() && i != size_t(-1); i += step) {
        if (doc->getPage(i)->isAnnotated()) {
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

void ScrollHandler::pageChanged(size_t page) {
    if (page == 0) {
        return;
    }
    jumpToPage(page - 1);
}
