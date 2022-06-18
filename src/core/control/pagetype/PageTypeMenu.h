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
    PageTypeInfo* info;
} MenuCallbackInfo;

enum class ApplyPageTypeSource {
    /** Apply page type selected in dialog */
    SELECTED,
    /** Apply page type of current page */
    CURRENT
};

class PageTypeMenuChangeListener {
public:
    virtual void changeCurrentPageBackground(PageTypeInfo* info) = 0;
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
    GtkWidget* getMenu();
    PageType getSelected();
    void loadDefaultPage();
    void setSelected(const PageType& selected);
    void setListener(PageTypeMenuChangeListener* listener);
    void hideCopyPage();

    /**
     * Apply background to current or to all pages button
     */
    void addApplyBackgroundButton(PageTypeApplyListener* pageTypeApplyListener, bool onlyAllMenu,
                                  ApplyPageTypeSource ptSource);

private:
    static GtkWidget* createApplyMenuItem(const char* text);
    void initDefaultMenu();
    void addMenuEntry(PageTypeInfo* t);
    void entrySelected(PageTypeInfo* t);
    cairo_surface_t* createPreviewImage(const PageType& pt);

private:
    bool showSpecial;

    GtkWidget* menu;
    PageTypeHandler* types;
    Settings* settings;

    std::vector<MenuCallbackInfo> menuInfos;

    PageType selected;

    bool ignoreEvents;

    ApplyPageTypeSource pageTypeSource;

    PageTypeMenuChangeListener* listener;

    guint menuX;
    guint menuY;

    bool showPreview;

    PageTypeApplyListener* pageTypeApplyListener;
};
