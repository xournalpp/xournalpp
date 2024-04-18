/*
 * Xournal++
 *
 * Dialog to configure page template for new pages
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr

#include "control/settings/PageTemplateSettings.h"  // for PageTemplateSettings
#include "util/raii/GtkWindowUPtr.h"

class PageTypeHandler;
class PageTypeInfo;
class GladeSearchpath;
class Settings;
class ToolMenuHandler;
class PageTypeSelectionPopoverGridOnly;

namespace xoj::popup {
class PageTemplateDialog final {
public:
    PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, ToolMenuHandler* toolmenu,
                       PageTypeHandler* types);
    PageTemplateDialog(PageTemplateDialog&) = delete;
    PageTemplateDialog(PageTemplateDialog&&) = delete;
    PageTemplateDialog& operator=(PageTemplateDialog&) = delete;
    PageTemplateDialog&& operator=(PageTemplateDialog&&) = delete;
    ~PageTemplateDialog();

public:
    /**
     * The dialog was confirmed / saved
     */
    bool isSaved() const;

    void changeCurrentPageBackground(const PageTypeInfo* info);

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    void showPageSizeDialog();
    void updatePageSize();
    void saveToFile();
    void loadFromFile();
    void updateDataFromModel();
    void saveToModel();

private:
    GladeSearchpath* gladeSearchPath;  // For opening subdialogs as needed
    xoj::util::GtkWindowUPtr window;

    Settings* settings;
    ToolMenuHandler* toolMenuHandler;
    PageTypeHandler* types;

    PageTemplateSettings model;

    std::unique_ptr<PageTypeSelectionPopoverGridOnly> pageTypeSelectionMenu;

    /**
     * The dialog was confirmed / saved
     */
    bool saved = false;

private:
    // Sub-widgets, owned by this->window
    GtkLabel* pageSizeLabel;
    GtkLabel* backgroundTypeLabel;
    GtkColorChooser* backgroundColorChooser;
    GtkCheckButton* copyLastPageButton;
    GtkCheckButton* copyLastPageSizeButton;
};
};  // namespace xoj::popup
