#include "ScrollHandler.h"

#include <memory>  // for __shared_ptr_access

#include <glib.h>  // for g_error

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
        scrollToPage(this->control->getDocument()->getPageCount() - 1);
    }
}

void ScrollHandler::goToFirstPage() {
    if (this->control->getWindow()) {
        scrollToPage(0);
    }
}

void ScrollHandler::scrollToPage(const PageRef& page, double top) {
    Document* doc = this->control->getDocument();

    doc->lock();
    auto p = doc->indexOf(page);
    doc->unlock();

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

void ScrollHandler::scrollToLinkDest(const LinkDestination& dest) {
    size_t pdfPage = dest.getPdfPage();
    Sidebar* sidebar = control->getSidebar();

    if (pdfPage != npos) {
        Document* doc = control->getDocument();
        doc->lock();
        size_t page = doc->findPdfPage(pdfPage);
        doc->unlock();

        if (page == npos) {
            sidebar->askInsertPdfPage(pdfPage);
        } else {
            if (dest.shouldChangeTop()) {
                control->getScrollHandler()->scrollToPage(page, dest.getTop() * control->getZoomControl()->getZoom());
            } else {
                if (control->getCurrentPageNo() != page) {
                    control->getScrollHandler()->scrollToPage(page);
                }
            }
        }
    }
}

void ScrollHandler::scrollToAnnotatedPage(bool next) {
    if (!this->control->getWindow()) {
        return;
    }

    int step = next ? 1 : -1;

    Document* doc = this->control->getDocument();

    for (size_t i = this->control->getCurrentPageNo() + step; i != npos && i < doc->getPageCount();
         i = ((i == 0 && step == -1) ? npos : i + step)) {
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

void ScrollHandler::pageChanged(size_t page) { scrollToSpinPage(); }
