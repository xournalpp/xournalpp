/*
 * Xournal++
 *
 * Handles page selection menu
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>
#include <vector>  // for vector

#include <cairo.h>    // for cairo_surface_t
#include <glib.h>     // for guint
#include <gtk/gtk.h>  // for GtkWidget

#include "model/PageType.h"  // for PageType

class PageTypeHandler;
class PageTypeInfo;
class Settings;

typedef struct {
    GtkWidget* entry;
    const PageTypeInfo* info;
} MenuCallbackInfo;

enum class ApplyPageTypeSource {
    /** Apply page type selected in dialog */
    SELECTED,
    /** Apply page type of current page */
    CURRENT
};

class PageTypeMenuChangeListener {
public:
    virtual void changeCurrentPageBackground(const PageTypeInfo* info) = 0;
    virtual ~PageTypeMenuChangeListener();
};

class PageTypeApplyListener {
public:
    virtual void applySelectedPageBackground(bool allPages, ApplyPageTypeSource src) = 0;
    virtual ~PageTypeApplyListener();
};

class PageTypeMenu {
public:
    PageTypeMenu(PageTypeHandler* types, Settings* settings, bool showPreview, bool showSpecial);

public:
    GtkWidget* getMenu() const;
    const std::optional<PageType>& getSelected() const;
    void loadDefaultPage();
    void setSelected(std::optional<PageType> selected);
    void setListener(PageTypeMenuChangeListener* listener);
    void hideCopyPage();

    /**
     * Apply background to current or to all pages button
     */
    void addApplyBackgroundButton(PageTypeApplyListener* pageTypeApplyListener, bool onlyAllMenu,
                                  ApplyPageTypeSource ptSource);

    /**
     * Create a small preview image of a specified page-type
     */
    static cairo_surface_t* createPreviewImage(const PageType& pt);

private:
    static GtkWidget* createApplyMenuItem(const char* text);
    void initDefaultMenu();
    void addMenuEntry(const PageTypeInfo* t);
    /// @brief Select the corresponding entry. If t == nullptr, the "Copy current page background" entry is selected.
    void entrySelected(const PageTypeInfo* t);

private:
    bool showSpecial;

    GtkWidget* menu;
    PageTypeHandler* types;
    Settings* settings;

    std::vector<MenuCallbackInfo> menuInfos;
    GtkWidget* copyCurrentBackgroundMenuEntry = nullptr;

    std::optional<PageType> selected;

    bool ignoreEvents;

    ApplyPageTypeSource pageTypeSource;

    PageTypeMenuChangeListener* listener;

    guint menuX;
    guint menuY;

    bool showPreview;

    PageTypeApplyListener* pageTypeApplyListener;

    // We need to be able to toggle the activation of the "Apply to Current Page"
    // entry when the page type changes, so this is a member variable.
    GtkWidget* menuEntryApply = nullptr;
};
