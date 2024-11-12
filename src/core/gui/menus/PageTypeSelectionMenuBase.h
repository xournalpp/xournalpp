/*
 * Xournal++
 *
 * Handles page selection menus -- Base
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <limits>
#include <optional>
#include <string_view>

#include <gio/gio.h>  // for GSimpleAction
#include <glib.h>

#include "model/PageType.h"  // for PageType
#include "util/raii/GObjectSPtr.h"

class PageTypeHandler;
class PageTypeInfo;
class Settings;

class PageTypeSelectionMenuBase {
protected:
    PageTypeSelectionMenuBase(PageTypeHandler* typesHandler, const Settings* settings,
                              const std::string_view& actionName);
    ~PageTypeSelectionMenuBase() = default;

public:
    /**
     * @brief Set the selected page type in the menu. Does not trigger any signals but updates the radio buttons.
     */
    void setSelectedPT(const std::optional<PageType>& selected);

private:
    /**
     * @brief callback subfunction
     */
    virtual void entrySelected(const PageTypeInfo* info) = 0;

    static void changeSelectionCallback(GSimpleAction* ga, GVariant* parameter,
                                        PageTypeSelectionMenuBase* pageTypeMenu);

protected:
    std::optional<PageType> selectedPT;

    xoj::util::GObjectSPtr<GSimpleAction> typeSelectionAction;

    PageTypeHandler* types;

    bool changeCurrentPageUponCallback = false;

public:
    static constexpr size_t COPY_CURRENT_PLACEHOLDER = std::numeric_limits<size_t>::max();
};
