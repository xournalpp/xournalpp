#include "ScrollHandler.h"

#include <memory>  // for __shared_ptr_access

#include <glib.h>  // for g_error

#include "control/settings/Settings.h"
#include "control/zoom/ZoomControl.h"  // for ZoomControl
#include "gui/Layout.h"
#include "gui/MainWindow.h"   // for MainWindow
#include "gui/XournalView.h"  // for XournalView
#include "gui/scroll/ScrollHandling.h"
#include "gui/sidebar/Sidebar.h"          // for Sidebar
#include "gui/widgets/SpinPageAdapter.h"  // for SpinPageAdapter
#include "gui/widgets/XournalWidget.h"
#include "model/Document.h"         // for Document
#include "model/LinkDestination.h"  // for LinkDestination
#include "model/XojPage.h"          // for XojPage
#include "util/Util.h"              // for npos
#include "util/i18n.h"

#include "Control.h"  // for Control

static constexpr int SCROLL_KEY_SIZE = 30;


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

void ScrollHandler::moveWhileInPresentationMode(ScrollHandler::Direction dir) {
    if (dir == LEFT || dir == UP) {
        goToPreviousPage();
    } else {
        goToNextPage();
    }
}

template <typename ArithmeticType>
static constexpr xoj::util::Point<ArithmeticType> getScroll(ScrollHandler::Direction dir, ArithmeticType amount) {
    return dir == ScrollHandler::LEFT  ? xoj::util::Point<ArithmeticType>(-amount, 0.) :
           dir == ScrollHandler::RIGHT ? xoj::util::Point<ArithmeticType>(amount, 0.) :
           dir == ScrollHandler::UP    ? xoj::util::Point<ArithmeticType>(0., -amount) :
                                         xoj::util::Point<ArithmeticType>(0., amount);
}

void ScrollHandler::scrollByOnePage(Direction dir) {
    if (control->getSettings()->isPresentationMode()) {
        moveWhileInPresentationMode(dir);
    } else {
        auto scroll = getScroll(dir, 1);
        control->getWindow()->getXournal()->pageRelativeXY(scroll.x, scroll.y);
    }
}

void ScrollHandler::scrollByOneStep(ScrollHandler::Direction dir) {
    if (control->getSettings()->isPresentationMode()) {
        moveWhileInPresentationMode(dir);
    } else {
        auto scroll = getScroll(dir, SCROLL_KEY_SIZE);
        control->getWindow()->getLayout()->scrollRelative(scroll.x, scroll.y);
    }
}

void ScrollHandler::scrollByVisibleArea(ScrollHandler::Direction dir) {
    if (control->getSettings()->isPresentationMode()) {
        moveWhileInPresentationMode(dir);
    } else {
        auto* scrollHandling = GTK_XOURNAL(control->getWindow()->getXournal()->getWidget())->scrollHandling;
        auto* adj = dir == UP || dir == DOWN ? scrollHandling->getVertical() : scrollHandling->getHorizontal();
        auto scroll = getScroll(dir, gtk_adjustment_get_page_size(adj) - SCROLL_KEY_SIZE);
        control->getWindow()->getLayout()->scrollRelative(scroll.x, scroll.y);
    }
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
                scrollToPage(page, {dest.getLeft(), dest.getTop(), -1, -1});
            } else {
                if (control->getCurrentPageNo() != page) {
                    scrollToPage(page);
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
    scrollToPage(page - 1);
}

std::ostream& operator<<(std::ostream& stream, ScrollHandler::Direction dir) {
    return stream << (dir == ScrollHandler::LEFT  ? C_("the direction", "Left") :
                      dir == ScrollHandler::RIGHT ? C_("the direction", "Right") :
                      dir == ScrollHandler::UP    ? C_("the direction", "Up") :
                                                    C_("the direction", "Down"));
}
