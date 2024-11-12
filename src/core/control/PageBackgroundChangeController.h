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
#include <functional>
#include <memory>  // for unique_ptr
#include <optional>
#include <variant>

#include "model/BackgroundImage.h"
#include "model/DocumentChangeType.h"  // for DocumentChangeType
#include "model/DocumentListener.h"    // for DocumentListener
#include "model/PageRef.h"             // for PageRef
#include "model/PageType.h"            // for PageType
#include "model/PaperSize.h"

#include "filesystem.h"  // for path

class Control;
class UndoAction;

class PageBackgroundChangeController: public DocumentListener {
public:
    PageBackgroundChangeController(Control* control);
    ~PageBackgroundChangeController() override = default;

public:
    void changeCurrentPageBackground(const PageType& pageType);
    void changeCurrentPageSize(const PaperSize& pageSize);
    /**
     * @brief (Un)set the page type for newly created pages
     * @param pageType The new page type.
     *      Passing std::nullopt will unset the page type.
     *      If the page type is not set, newly created pages will have the same type as the current page.
     */
    void setPageTypeForNewPages(const std::optional<PageType>& pageType);
    /**
     * @brief (Un)set the paper size for newly created pages
     * @param paperSize The new paper size.
     *      Passing std::nullopt will unset the paper size.
     *      If the page size is not set, newly created pages will have the same format as the current page.
     */
    void setPaperSizeForNewPages(const std::optional<PaperSize>& paperSize);
    void applyCurrentPageBackgroundToAll();
    void applyBackgroundToAllPages(const PageType& pt);
    void applyPageSizeToAllPages(const PaperSize& paperSize);
    void changePdfPagesBackground(const fs::path& filepath, bool attachPdf);
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

    /// Asks the user to choose a pdf page, and calls callback
    void askForPdfBackground(std::function<void(size_t)> callback);

    /// Asks the user to choose an image background, and calls callback
    void askForImageBackground(std::function<void(BackgroundImage)> callback);

    /// Nothing, a BackgroundImage or the number of a pdf page
    using CommitParameter = std::variant<std::nullopt_t, BackgroundImage, size_t>;
    /**
     * Applies pageType to page number pageNum.
     *      If pageType.isImage(), a BackgroundImage must be provided
     *      If pageType.isPdf(), a pdf page number must be provided in the std::variant
     */
    auto commitPageTypeChange(size_t pageNum, const PageType& pageType, CommitParameter param = std::nullopt)
            -> std::unique_ptr<UndoAction>;

    /**
     * Perform page size change.
     * @param pageNum Index of the page whose size will be changed
     * @param pageSize New page size
     * @return Undo action of the page size change
     */
    auto commitPageSizeChange(size_t pageNum, const PaperSize& pageSize) -> std::unique_ptr<UndoAction>;

private:
    Control* control = nullptr;
    std::optional<PageType> pageTypeForNewPages;
    std::optional<PaperSize> paperSizeForNewPages;
};
