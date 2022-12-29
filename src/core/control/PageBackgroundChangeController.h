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

#include <gtk/gtk.h>  // for GtkWidget

#include "control/pagetype/PageTypeMenu.h"  // for ApplyPageTypeSource, Page...
#include "model/DocumentChangeType.h"       // for DocumentChangeType
#include "model/DocumentListener.h"         // for DocumentListener
#include "model/PageRef.h"                  // for PageRef

class Control;
class UndoAction;
class PageType;
class PageTypeInfo;

class PageBackgroundChangeController:
        public PageTypeMenuChangeListener,
        public DocumentListener,
        public PageTypeApplyListener {
public:
    PageBackgroundChangeController(Control* control);
    ~PageBackgroundChangeController() override = default;

public:
    virtual void changeCurrentPageBackground(PageType& pageType);
    void changeCurrentPageBackground(PageTypeInfo* info) override;
    void changeAllPagesBackground(const PageType& pt);
    void insertNewPage(size_t position, bool shouldScrollToPage = true);
    GtkWidget* getMenu();

    // DocumentListener
public:
    void documentChanged(DocumentChangeType type) override;
    void pageSizeChanged(size_t page) override;
    void pageChanged(size_t page) override;
    void pageInserted(size_t page) override;
    void pageDeleted(size_t page) override;
    void pageSelected(size_t page) override;

    // PageTypeApplyListener
public:
    void applySelectedPageBackground(bool allPages, ApplyPageTypeSource src) override;

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
    PageTypeMenu currentPageType;
    bool ignoreEvent = false;
};
