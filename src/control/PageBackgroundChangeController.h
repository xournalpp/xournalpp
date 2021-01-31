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

#include <string>
#include <vector>

#include "control/pagetype/PageTypeMenu.h"
#include "control/settings/PageTemplateSettings.h"
#include "model/DocumentListener.h"
#include "model/PageRef.h"

#include "XournalType.h"

class PageTypeMenu;
class Control;
class XojPage;
class UndoAction;

class PageBackgroundChangeController:
        public PageTypeMenuChangeListener,
        public DocumentListener,
        public PageTypeApplyListener {
public:
    PageBackgroundChangeController(Control* control);
    virtual ~PageBackgroundChangeController() = default;

public:
    virtual void changeCurrentPageBackground(PageType& pageType);
    virtual void changeCurrentPageBackground(PageTypeInfo* info);
    void changeAllPagesBackground(const PageType& pt);
    void insertNewPage(size_t position);
    GtkWidget* getMenu();

    // DocumentListener
public:
    virtual void documentChanged(DocumentChangeType type);
    virtual void pageSizeChanged(size_t page);
    virtual void pageChanged(size_t page);
    virtual void pageInserted(size_t page);
    virtual void pageDeleted(size_t page);
    virtual void pageSelected(size_t page);

    // PageTypeApplyListener
public:
    virtual void applySelectedPageBackground(bool allPages, ApplyPageTypeSource src);

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
