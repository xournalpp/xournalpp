#include "PageTypeMenu.h"

#include <algorithm>  // for max
#include <memory>     // for unique_ptr
#include <string>     // for string

#include <glib-object.h>       // for G_CALLBACK, g_sig...
#include <util/gtk4_helper.h>  // for gtk_box_append

#include "control/settings/PageTemplateSettings.h"  // for PageTemplateSettings
#include "control/settings/Settings.h"              // for Settings
#include "gui/CreatePreviewImage.h"                 // for CreatePreviewImage
#include "util/Assert.h"                            // or xoj_assert
#include "util/Color.h"                             // for Color
#include "util/i18n.h"                              // for _
#include "util/raii/CairoWrappers.h"                // for CairoSurfaceSPtr
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

void PageTypeMenu::addMenuEntry(const PageTypeInfo* t) {
    bool special = t == nullptr || t->page.isSpecial();
    bool showImg = !special && showPreview;

    GtkWidget* entry = nullptr;
    if (showImg) {
        GtkWidget* preview = xoj::helper::createPreviewImage(t->page);
        entry = gtk_check_menu_item_new();

        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_box_append(GTK_BOX(box), preview);
        gtk_box_append(GTK_BOX(box), gtk_label_new(t->name.c_str()));

        gtk_container_add(GTK_CONTAINER(entry), box);
        gtk_widget_show_all(entry);
    } else {
        entry = gtk_check_menu_item_new_with_label(t ? t->name.c_str() : _("Copy current"));
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

    if (t) {
        menuInfos.push_back({entry, t});
    } else {
        copyCurrentBackgroundMenuEntry = entry;
    }

    g_signal_connect(entry, "toggled", G_CALLBACK(+[](GtkWidget* togglebutton, PageTypeMenu* self) {
                         if (self->ignoreEvents) {
                             return;
                         }

                         if (togglebutton == self->copyCurrentBackgroundMenuEntry) {
                             self->entrySelected(nullptr);
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

void PageTypeMenu::entrySelected(const PageTypeInfo* t) {
    ignoreEvents = true;
    for (MenuCallbackInfo& info: menuInfos) {
        bool enabled = info.info == t;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(info.entry), enabled);
    }
    if (copyCurrentBackgroundMenuEntry) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(copyCurrentBackgroundMenuEntry), t == nullptr);
    }
    ignoreEvents = false;

    selected = t ? std::make_optional(t->page) : std::nullopt;

    if (listener != nullptr) {
        listener->changeCurrentPageBackground(t);
    }

    // Disable "Apply to current page" if current format is "Copy."
    if (this->menuEntryApply) {
        gtk_widget_set_sensitive(this->menuEntryApply, t != nullptr);
    }
}

void PageTypeMenu::setSelected(std::optional<PageType> selected) {
    if (selected) {
        entrySelected(this->types->getInfoOn(selected.value()));
    } else {
        entrySelected(nullptr);
    }
}

void PageTypeMenu::setListener(PageTypeMenuChangeListener* listener) { this->listener = listener; }

void PageTypeMenu::hideCopyPage() { gtk_widget_hide(copyCurrentBackgroundMenuEntry); }

/**
 * Apply background to current or to all pages button
 */
void PageTypeMenu::addApplyBackgroundButton(PageTypeApplyListener* pageTypeApplyListener, bool onlyAllMenu,
                                            ApplyPageTypeSource ptSource) {
    xoj_assert(!this->menuEntryApply);

    this->pageTypeApplyListener = pageTypeApplyListener;
    this->pageTypeSource = ptSource;

    GtkWidget* separator = gtk_separator_menu_item_new();
    gtk_widget_show(separator);

    gtk_menu_attach(GTK_MENU(menu), separator, 0, PREVIEW_COLUMNS, menuY, menuY + 1);
    menuY++;

    if (!onlyAllMenu) {
        this->menuEntryApply = createApplyMenuItem(_("Apply to current page"));
        gtk_menu_attach(GTK_MENU(menu), menuEntryApply, 0, PREVIEW_COLUMNS, menuY, menuY + 1);
        menuY++;
        g_signal_connect(menuEntryApply, "activate", G_CALLBACK(+[](GtkWidget* menu, PageTypeMenu* self) {
                             self->pageTypeApplyListener->applySelectedPageBackground(false, self->pageTypeSource);
                         }),
                         this);
        // Do not initially activate this option if the "Copy" format is selected
        if (!selected) {
            gtk_widget_set_sensitive(menuEntryApply, false);
        }
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
    for (auto& t: this->types->getPageTypes()) {
        addMenuEntry(t.get());
    }
    if (showSpecial) {
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
        for (auto& t: this->types->getSpecialPageTypes()) {
            addMenuEntry(t.get());
        }
        addMenuEntry(nullptr);  // Copy button
    }
}

auto PageTypeMenu::getMenu() const -> GtkWidget* { return menu; }

auto PageTypeMenu::getSelected() const -> const std::optional<PageType>& { return selected; }
