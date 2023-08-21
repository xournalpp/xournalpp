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

#include <gtk/gtk.h>  // for GtkWindow

#include "control/settings/PageTemplateSettings.h"  // for PageTemplateSettings
#include "gui/GladeGui.h"                           // for GladeGui

class PageTypeHandler;
class PageTypeInfo;
class PopupMenuButton;
class GladeSearchpath;
class Settings;
class PageTypeSelectionPopoverGridOnly;

class PageTemplateDialog: public GladeGui {
public:
    PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, PageTypeHandler* types);
    PageTemplateDialog(PageTemplateDialog&) = delete;
    PageTemplateDialog(PageTemplateDialog&&) = delete;
    PageTemplateDialog& operator=(PageTemplateDialog&) = delete;
    PageTemplateDialog&& operator=(PageTemplateDialog&&) = delete;
    ~PageTemplateDialog() override;

public:
    void show(GtkWindow* parent) override;

    /**
     * The dialog was confirmed / saved
     */
    bool isSaved() const;

    void changeCurrentPageBackground(const PageTypeInfo* info);

private:
    void showPageSizeDialog();
    void updatePageSize();
    void saveToFile();
    void loadFromFile();
    void updateDataFromModel();
    void saveToModel();

private:
    Settings* settings;
    PageTypeHandler* types;

    PageTemplateSettings model;

    std::unique_ptr<PageTypeSelectionPopoverGridOnly> pageTypeSelectionMenu;

    std::unique_ptr<PopupMenuButton> popupMenuButton;

    /**
     * The dialog was confirmed / saved
     */
    bool saved = false;
};
