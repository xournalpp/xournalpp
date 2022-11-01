#include "PageTypeMenu.h"

#include <algorithm>  // for max
#include <memory>     // for unique_ptr
#include <string>     // for string

#include <glib-object.h>       // for G_CALLBACK, g_sig...
#include <util/gtk4_helper.h>  // for gtk_box_append

#include "control/settings/PageTemplateSettings.h"  // for PageTemplateSettings
#include "control/settings/Settings.h"              // for Settings
#include "util/Color.h"                             // for Color
#include "util/i18n.h"                              // for _
#include "view/background/BackgroundView.h"         // for BackgroundView

#include "PageTypeHandler.h"  // for PageTypeInfo, Pag...

PageTypeMenuChangeListener::~PageTypeMenuChangeListener() = default;
PageTypeApplyListener::~PageTypeApplyListener() = default;

#define PREVIEW_COLUMNS 3


PageTypeMenu::PageTypeMenu(PageTypeHandler* types, Settings* settings, bool showPreview, bool showSpecial):
        showSpecial(showSpecial),
        menu(gtk_menu_new()),
        types(types),
        settings(settings),
        ignoreEvents(false),
        pageTypeSource(ApplyPageTypeSource::SELECTED),
        listener(nullptr),
        menuX(0),
        menuY(0),
        showPreview(showPreview),
        pageTypeApplyListener(nullptr) {
    initDefaultMenu();
    loadDefaultPage();
}

void PageTypeMenu::loadDefaultPage() {
    PageTemplateSettings model;
    model.parse(settings->getPageTemplate());
    setSelected(model.getPageInsertType());
}

auto PageTypeMenu::createPreviewImage(const PageType& pt) -> cairo_surface_t* {
    const int previewWidth = 100;
    const int previewHeight = 141;
    const double zoom = 0.5;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, previewWidth, previewHeight);
    cairo_t* cr = cairo_create(surface);
    cairo_scale(cr, zoom, zoom);

    auto bgView = xoj::view::BackgroundView::createRuled(previewWidth / zoom, previewHeight / zoom, Colors::white,
                                                         pt, 2.0);
    bgView->draw(cr);

    cairo_identity_matrix(cr);

    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, previewWidth, 0);
    cairo_line_to(cr, previewWidth, previewHeight);
    cairo_line_to(cr, 0, previewHeight);
    cairo_line_to(cr, 0, 0);
    cairo_stroke(cr);

    cairo_destroy(cr);
    return surface;
}

void PageTypeMenu::addMenuEntry(PageTypeInfo* t) {
    bool special = t->page.isSpecial();
    bool showImg = !special && showPreview;

    GtkWidget* entry = nullptr;
    if (showImg) {
        cairo_surface_t* img = createPreviewImage(t->page);
        GtkWidget* preview = gtk_image_new_from_surface(img);
        entry = gtk_check_menu_item_new();

        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_box_append(GTK_BOX(box), preview);
        gtk_box_append(GTK_BOX(box), gtk_label_new(t->name.c_str()));

        gtk_container_add(GTK_CONTAINER(entry), box);
        gtk_widget_show_all(entry);
    } else {
        entry = gtk_check_menu_item_new_with_label(t->name.c_str());
        gtk_widget_show(entry);
        gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(entry), true);
    }

    if (showPreview) {
        if (special) {
            if (menuX != 0) {
                menuX = 0;
                menuY++;
            }

            gtk_menu_attach(GTK_MENU(menu), entry, menuX, menuX + PREVIEW_COLUMNS, menuY, menuY + 1);
            menuY++;
        } else {
            gtk_menu_attach(GTK_MENU(menu), entry, menuX, menuX + 1, menuY, menuY + 1);
            menuX++;
            if (menuX >= PREVIEW_COLUMNS) {
                menuX = 0;
                menuY++;
            }
        }
    } else {
        gtk_container_add(GTK_CONTAINER(menu), entry);
    }

    gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(entry), true);

    MenuCallbackInfo info;
    info.entry = entry;
    info.info = t;
    menuInfos.push_back(info);

    g_signal_connect(entry, "toggled", G_CALLBACK(+[](GtkWidget* togglebutton, PageTypeMenu* self) {
                         if (self->ignoreEvents) {
                             return;
                         }

                         for (MenuCallbackInfo& info: self->menuInfos) {
                             if (info.entry == togglebutton) {
                                 self->entrySelected(info.info);
                                 break;
                             }
                         }
                     }),
                     this);
}

void PageTypeMenu::entrySelected(PageTypeInfo* t) {
    ignoreEvents = true;
    for (MenuCallbackInfo& info: menuInfos) {
        bool enabled = info.info == t;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(info.entry), enabled);
    }
    ignoreEvents = false;

    selected = t->page;

    if (listener != nullptr) {
        listener->changeCurrentPageBackground(t);
    }
}

void PageTypeMenu::setSelected(const PageType& selected) {
    for (MenuCallbackInfo& info: menuInfos) {
        if (info.info->page == selected) {
            entrySelected(info.info);
            break;
        }
    }
}

void PageTypeMenu::setListener(PageTypeMenuChangeListener* listener) { this->listener = listener; }

void PageTypeMenu::hideCopyPage() {
    for (MenuCallbackInfo& info: menuInfos) {
        if (info.info->page.format == PageTypeFormat::Copy) {
            gtk_widget_hide(info.entry);
            break;
        }
    }
}

/**
 * Apply background to current or to all pages button
 */
void PageTypeMenu::addApplyBackgroundButton(PageTypeApplyListener* pageTypeApplyListener, bool onlyAllMenu,
                                            ApplyPageTypeSource ptSource) {
    this->pageTypeApplyListener = pageTypeApplyListener;
    this->pageTypeSource = ptSource;

    GtkWidget* separator = gtk_separator_menu_item_new();
    gtk_widget_show(separator);

    gtk_menu_attach(GTK_MENU(menu), separator, 0, PREVIEW_COLUMNS, menuY, menuY + 1);
    menuY++;

    if (!onlyAllMenu) {
        GtkWidget* menuEntryApply = createApplyMenuItem(_("Apply to current page"));
        gtk_menu_attach(GTK_MENU(menu), menuEntryApply, 0, PREVIEW_COLUMNS, menuY, menuY + 1);
        menuY++;
        g_signal_connect(menuEntryApply, "activate", G_CALLBACK(+[](GtkWidget* menu, PageTypeMenu* self) {
                             self->pageTypeApplyListener->applySelectedPageBackground(false, self->pageTypeSource);
                         }),
                         this);
    }

    GtkWidget* menuEntryApplyAll = createApplyMenuItem(_("Apply to all pages"));
    gtk_menu_attach(GTK_MENU(menu), menuEntryApplyAll, 0, PREVIEW_COLUMNS, menuY, menuY + 1);
    menuY++;
    g_signal_connect(menuEntryApplyAll, "activate", G_CALLBACK(+[](GtkWidget* menu, PageTypeMenu* self) {
                         self->pageTypeApplyListener->applySelectedPageBackground(true, self->pageTypeSource);
                     }),
                     this);
}

auto PageTypeMenu::createApplyMenuItem(const char* text) -> GtkWidget* {
    GtkWidget* menuItem = gtk_menu_item_new();
    gtk_menu_item_set_label(GTK_MENU_ITEM(menuItem), text);
    gtk_widget_show_all(menuItem);
    return menuItem;
}

void PageTypeMenu::initDefaultMenu() {
    bool special = false;
    for (PageTypeInfo* t: this->types->getPageTypes()) {
        if (!showSpecial && t->page.isSpecial()) {
            continue;
        }

        if (!special && t->page.isSpecial()) {
            special = true;
            GtkWidget* separator = gtk_separator_menu_item_new();
            gtk_widget_show(separator);

            if (showPreview) {
                if (menuX != 0) {
                    menuX = 0;
                    menuY++;
                }

                gtk_menu_attach(GTK_MENU(menu), separator, menuX, menuX + PREVIEW_COLUMNS, menuY, menuY + 1);
                menuY++;
            } else {
                gtk_container_add(GTK_CONTAINER(menu), separator);
            }
        }
        addMenuEntry(t);
    }
}

auto PageTypeMenu::getMenu() -> GtkWidget* { return menu; }

auto PageTypeMenu::getSelected() -> PageType { return selected; }
