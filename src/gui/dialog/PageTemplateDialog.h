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

#include "control/pagetype/PageTypeMenu.h"
#include "control/settings/PageTemplateSettings.h"
#include "control/settings/Settings.h"
#include "gui/GladeGui.h"

class PageTypeHandler;
class PageTypeMenu;
class PageTypeInfo;
class PopupMenuButton;

class PageTemplateDialog: public GladeGui, public PageTypeMenuChangeListener {
public:
    PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, PageTypeHandler* types);
    PageTemplateDialog(PageTemplateDialog&) = delete;
    PageTemplateDialog(PageTemplateDialog&&) = delete;
    PageTemplateDialog& operator=(PageTemplateDialog&) = delete;
    PageTemplateDialog&& operator=(PageTemplateDialog&&) = delete;
    virtual ~PageTemplateDialog();

public:
    virtual void show(GtkWindow* parent);

    /**
     * The dialog was confirmed / saved
     */
    bool isSaved() const;

    void changeCurrentPageBackground(PageTypeInfo* info);

private:
    void showPageSizeDialog();
    void updatePageSize();
    void saveToFile();
    void loadFromFile();
    void updateDataFromModel();
    void saveToModel();

private:
    Settings* settings;

    PageTemplateSettings model;

    std::unique_ptr<PageTypeMenu> pageMenu;

    std::unique_ptr<PopupMenuButton> popupMenuButton;

    /**
     * The dialog was confirmed / saved
     */
    bool saved = false;
};
