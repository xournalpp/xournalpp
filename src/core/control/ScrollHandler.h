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
#include "pdf/base/XojPdfPage.h"          // for XojPdfRectangle

class Control;
class LinkDestination;

class ScrollHandler: public SpinPageListener {
public:
    ScrollHandler(Control* control);
    ~ScrollHandler() override;

    enum Direction { LEFT, RIGHT, DOWN, UP };

public:
    void goToPreviousPage();
    void goToNextPage();

    void goToLastPage();
    void goToFirstPage();

    void scrollToPage(const PageRef& page, XojPdfRectangle rect = {0, 0, -1, -1});
    void scrollToPage(size_t page, XojPdfRectangle rect = {0, 0, -1, -1});

    void scrollToAnnotatedPage(bool next);

    void scrollByOnePage(Direction dir);      ///< Move to the next page of the document (in direction dir)
    void scrollByOneStep(Direction dir);      ///< Move by a small amount (in direction dir)
    void scrollByVisibleArea(Direction dir);  ///< Move by the height or width of the visible area (- a small overlap)

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
    void moveWhileInPresentationMode(Direction dir);

private:
    Control* control = nullptr;
};
