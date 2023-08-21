/*
 * Xournal++
 *
 * Handle page background change and all other PageBackground stuff
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <optional>

#include "model/DocumentChangeType.h"       // for DocumentChangeType
#include "model/DocumentListener.h"         // for DocumentListener
#include "model/PageRef.h"                  // for PageRef
#include "model/PageType.h"

class Control;
class UndoAction;

class PageBackgroundChangeController: public DocumentListener {
public:
    PageBackgroundChangeController(Control* control);
    ~PageBackgroundChangeController() override = default;

public:
    void changeCurrentPageBackground(const PageType& pageType);
    /**
     * @brief (Un)set the page type for newly created pages
     * @param pageType The new page type.
     *      Passing std::nullopt will unset the page type.
     *      If the page type is not set, newly created pages will have the same type as the current page.
     */
    void setPageTypeForNewPages(const std::optional<PageType>& pageType);
    void applyCurrentPageBackgroundToAll();
    void applyBackgroundToAllPages(const PageType& pt);
    void insertNewPage(size_t position, bool shouldScrollToPage = true);

    // DocumentListener
public:
    void documentChanged(DocumentChangeType type) override;
    void pageSizeChanged(size_t page) override;
    void pageChanged(size_t page) override;
    void pageInserted(size_t page) override;
    void pageDeleted(size_t page) override;
    void pageSelected(size_t page) override;

private:
    /**
     * Copy the background from source to target
     */
    static void copyBackgroundFromOtherPage(PageRef target, PageRef source);

    /**
     * Apply the background to the page, asks for PDF Page or Image, if needed
     *
     * @return true on success, false if the user cancels
     */
    bool applyPageBackground(PageRef page, const PageType& pt);

    /**
     * Apply a new PDF Background, asks the user which page should be selected
     *
     * @return true on success, false if the user cancels
     */
    bool applyPdfBackground(PageRef page);

    /**
     * Apply a new Image Background, asks the user which image should be inserted
     *
     * @return true on success, false if the user cancels
     */
    bool applyImageBackground(PageRef page);

    /**
     * Perform the page type change.
     */
    auto commitPageTypeChange(size_t pageNum, const PageType& pageType) -> std::unique_ptr<UndoAction>;

private:
    Control* control = nullptr;
    std::optional<PageType> pageTypeForNewPages;
};
