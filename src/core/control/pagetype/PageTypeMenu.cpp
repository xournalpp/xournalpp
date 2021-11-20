#include "PageTypeMenu.h"

#include <cairo.h>
#include <gio/gmenu.h>

#include "control/settings/PageTemplateSettings.h"
#include "control/settings/Settings.h"
#include "util/i18n.h"
#include "view/background/MainBackgroundPainter.h"

#include "PageTypeHandler.h"

#define PREVIEW_COLUMNS 3


PageTypeMenu::PageTypeMenu(PageTypeHandler* types, Settings* settings, bool showPreview, bool showSpecial):
        showSpecial(showSpecial),
        menu(g_menu_new()),
        types(types),
        settings(settings),
        ignoreEvents(false),
        listener(nullptr),
        menuX(0),
        menuY(0),
        showPreview(showPreview),
        pageTypeApplyListener(nullptr),
        pageTypeSource(ApplyPageTypeSource::SELECTED) {
    initDefaultMenu();
    loadDefaultPage();
}

void PageTypeMenu::loadDefaultPage() {
    PageTemplateSettings model;
    model.parse(settings->getPageTemplate());
    setSelected(model.getPageInsertType());
}

auto PageTypeMenu::createPreviewImage(MainBackgroundPainter* bgPainter, const PageType& pt) -> cairo_surface_t* {
    int previewWidth = 100;
    int previewHeight = 141;
    double zoom = 0.5;

    auto page = std::make_shared<XojPage>(previewWidth / zoom, previewHeight / zoom);

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, previewWidth, previewHeight);
    cairo_t* cr = cairo_create(surface);
    cairo_scale(cr, zoom, zoom);

    bgPainter->paint(pt, cr, std::move(page));

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

auto PageTypeMenu::createMenuEntry(MainBackgroundPainter* bgPainter, PageTypeInfo* t) -> GMenuItem* {
    GMenuItem* entry = g_menu_item_new(t->name.c_str(), nullptr);
    bool special = t->page.isSpecial();
    bool showImg = !special && showPreview;
    if (showImg) {
        cairo_surface_t* img = createPreviewImage(bgPainter, t->page);
        auto width = cairo_image_surface_get_stride(img);
        auto height = cairo_image_surface_get_height(img);
        auto* data = cairo_image_surface_get_data(img);
        auto* g_bytes = g_bytes_new_with_free_func(data, width * height, GDestroyNotify(&cairo_surface_destroy), img);
        auto* icon = g_bytes_icon_new(g_bytes);
        g_menu_item_set_icon(entry, icon);
        g_bytes_unref(g_bytes);
    }

    MenuCallbackInfo info;
    info.entry = entry;
    info.info = t;
    menuInfos.push_back(info);
    return entry;

    // g_signal_connect(entry, "toggled", G_CALLBACK(+[](GtkWidget* togglebutton, PageTypeMenu* self) {
    //                      if (self->ignoreEvents) {
    //                          return;
    //                      }

    //                      for (MenuCallbackInfo& info: self->menuInfos) {
    //                          if (info.entry == togglebutton) {
    //                              self->entrySelected(info.info);
    //                              break;
    //                          }
    //                      }
    //                  }),
    //                  this);
}

void PageTypeMenu::entrySelected(PageTypeInfo* t) {
    ignoreEvents = true;
    for (MenuCallbackInfo& info: menuInfos) {
        bool enabled = info.info == t;
        // TODO(gtk4): fix this
        // gtk_check_button_set_active(GTK_CHECK_BUTTON(info.entry), enabled);
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
            // Todo (gtk4, fabian): remove item from list
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

    GMenu* apply_menu = g_menu_new();
    if (!onlyAllMenu) {
        g_menu_append(apply_menu, _("Apply to current page"), nullptr);
        // g_signal_connect(menuEntryApply, "activate", G_CALLBACK(+[](GtkWidget* menu, PageTypeMenu* self) {
        //                      self->pageTypeApplyListener->applySelectedPageBackground(false, self->pageTypeSource);
        //                  }),
        //                  this);
    }

    g_menu_append(apply_menu, _("Apply to all pages"), nullptr);
    // g_signal_connect(menuEntryApplyAll, "activate", G_CALLBACK(+[](GtkWidget* menu, PageTypeMenu* self) {
    //                      self->pageTypeApplyListener->applySelectedPageBackground(true, self->pageTypeSource);
    //                  }),
    //                  this);
    g_menu_append_section(menu, nullptr, G_MENU_MODEL(apply_menu));
    // g_free(apply_menu);
}

auto PageTypeMenu::createApplyMenuItem(const char* text) -> GMenuItem* {
    auto* menuItem = g_menu_item_new(text, nullptr);
    return menuItem;
}

void PageTypeMenu::initDefaultMenu() {
    auto bgPainter = std::make_unique<MainBackgroundPainter>();
    bgPainter->setLineWidthFactor(2);

    GMenu* menu = g_menu_new();
    GMenu* menu_special = showSpecial ? g_menu_new() : nullptr;
    //
    for (PageTypeInfo* t: this->types->getPageTypes()) {
        auto special = t->page.isSpecial();
        if (!showSpecial && special) {
            continue;
        }
        g_menu_append_item((!special ? menu : menu_special), createMenuEntry(bgPainter.get(), t));
    }
    // \\f
    if (showSpecial)
        g_menu_append_section(menu, nullptr, G_MENU_MODEL(menu_special));
}

auto PageTypeMenu::getMenu() -> GMenu* { return menu; }

auto PageTypeMenu::getSelected() -> PageType { return selected; }
