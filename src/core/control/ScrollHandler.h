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

#include <cstddef>  // for size_t

#include "gui/widgets/SpinPageAdapter.h"  // for SpinPageListener
#include "model/PageRef.h"                // for PageRef

class Control;
class LinkDestination;

class ScrollHandler: public SpinPageListener {
public:
    ScrollHandler(Control* control);
    ~ScrollHandler() override;

public:
    void goToPreviousPage();
    void goToNextPage();

    void goToLastPage();
    void goToFirstPage();

    void scrollToPage(const PageRef& page, double top = 0);
    void scrollToPage(size_t page, double top = 0);

    void scrollToAnnotatedPage(bool next);

    /**
     * Scroll to a given link's destination, provided the
     * destination is a local destination and not a URI.
     *
     *  If the destination is a non-existent PDF page,
     * we ask the user whether to add the missing page or not.
     *
     * @param dest is to shown
     */
    void scrollToLinkDest(const LinkDestination& dest);

    bool isPageVisible(size_t page, int* visibleHeight = nullptr);

public:
    void pageChanged(size_t page) override;

private:
    void scrollToSpinPage();

private:
    Control* control = nullptr;
};
