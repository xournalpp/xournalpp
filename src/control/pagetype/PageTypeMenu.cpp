#include "PageTypeMenu.h"

#include "control/settings/PageTemplateSettings.h"
#include "control/settings/Settings.h"
#include "view/background/MainBackgroundPainter.h"

#include "PageTypeHandler.h"
#include "i18n.h"

PageTypeMenuChangeListener::~PageTypeMenuChangeListener() = default;
PageTypeApplyListener::~PageTypeApplyListener() = default;

#define PREVIEW_COLUMNS 3


PageTypeMenu::PageTypeMenu(PageTypeHandler* types, Settings* settings, bool showPreview, bool showSpecial):
        showSpecial(showSpecial),
        menu(gtk_menu_new()),
        types(types),
        settings(settings),
        ignoreEvents(false),
        listener(nullptr),
        menuX(0),
        menuY(0),
        backgroundPainter(nullptr),
        showPreview(showPreview),
        pageTypeApplyListener(nullptr) {
    initDefaultMenu();
    loadDefaultPage();
}

PageTypeMenu::~PageTypeMenu() {
    /**
     * The menu is used from the GUI
     * Therefore the menu is not freed here, this will be done in the GUI
     */
    menu = nullptr;
}

void PageTypeMenu::loadDefaultPage() {
    PageTemplateSettings model;
    model.parse(settings->getPageTemplate());
    setSelected(model.getPageInsertType());
}

auto PageTypeMenu::createPreviewImage(const PageType& pt) -> cairo_surface_t* {
    int previewWidth = 100;
    int previewHeight = 141;
    double zoom = 0.5;

    auto page = std::make_shared<XojPage>(previewWidth / zoom, previewHeight / zoom);

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, previewWidth, previewHeight);
    cairo_t* cr = cairo_create(surface);
    cairo_scale(cr, zoom, zoom);

    backgroundPainter->paint(pt, cr, std::move(page));

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

        gtk_container_add(GTK_CONTAINER(box), preview);
        gtk_container_add(GTK_CONTAINER(box), gtk_label_new(t->name.c_str()));

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
void PageTypeMenu::addApplyBackgroundButton(PageTypeApplyListener* pageTypeApplyListener, bool onlyAllMenu) {
    this->pageTypeApplyListener = pageTypeApplyListener;

    GtkWidget* separator = gtk_separator_menu_item_new();
    gtk_widget_show(separator);

    gtk_menu_attach(GTK_MENU(menu), separator, 0, PREVIEW_COLUMNS, menuY, menuY + 1);
    menuY++;

    if (!onlyAllMenu) {
        GtkWidget* menuEntryApply = createApplyMenuItem(_("Apply to current page"));
        gtk_menu_attach(GTK_MENU(menu), menuEntryApply, 0, PREVIEW_COLUMNS, menuY, menuY + 1);
        menuY++;
        g_signal_connect(menuEntryApply, "activate", G_CALLBACK(+[](GtkWidget* menu, PageTypeMenu* self) {
                             self->pageTypeApplyListener->applyCurrentPageBackground(false);
                         }),
                         this);
    }

    GtkWidget* menuEntryApplyAll = createApplyMenuItem(_("Apply to all pages"));
    gtk_menu_attach(GTK_MENU(menu), menuEntryApplyAll, 0, PREVIEW_COLUMNS, menuY, menuY + 1);
    menuY++;
    g_signal_connect(menuEntryApplyAll, "activate", G_CALLBACK(+[](GtkWidget* menu, PageTypeMenu* self) {
                         self->pageTypeApplyListener->applyCurrentPageBackground(true);
                     }),
                     this);
}

auto PageTypeMenu::createApplyMenuItem(const char* text) -> GtkWidget* {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget* icon = gtk_image_new_from_icon_name("gtk-apply", GTK_ICON_SIZE_MENU);
    GtkWidget* label = gtk_label_new(text);
    GtkWidget* menuItem = gtk_menu_item_new();

    gtk_container_add(GTK_CONTAINER(box), icon);
    gtk_container_add(GTK_CONTAINER(box), label);

    gtk_container_add(GTK_CONTAINER(menuItem), box);

    gtk_widget_show_all(menuItem);

    return menuItem;
}

void PageTypeMenu::initDefaultMenu() {
    this->backgroundPainter = new MainBackgroundPainter();
    this->backgroundPainter->setLineWidthFactor(2);

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

    delete this->backgroundPainter;
    this->backgroundPainter = nullptr;
}

auto PageTypeMenu::getMenu() -> GtkWidget* { return menu; }

auto PageTypeMenu::getSelected() -> PageType { return selected; }
