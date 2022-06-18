#include "MainWindowToolbarMenu.h"

#include <string>  // for operator==, string

#include <glib-object.h>  // for G_CALLBACK, g_sig...

#include "control/settings/Settings.h"              // for Settings
#include "gui/toolbarMenubar/ToolMenuHandler.h"     // for ToolMenuHandler
#include "gui/toolbarMenubar/model/ToolbarData.h"   // for ToolbarData
#include "gui/toolbarMenubar/model/ToolbarModel.h"  // for ToolbarModel

#include "MainWindow.h"  // for MainWindow

class MenuSelectToolbarData {
public:
    MenuSelectToolbarData(MainWindowToolbarMenu* tbm, GtkWidget* item, ToolbarData* d, int index):
            tbm(tbm), item(item), d(d), index(index) {}

    MainWindowToolbarMenu* tbm;
    GtkWidget* item;
    ToolbarData* d;
    int index;
};


MainWindowToolbarMenu::MainWindowToolbarMenu(MainWindow* win): win(win) {}

MainWindowToolbarMenu::~MainWindowToolbarMenu() {
    menuitems.clear();
    freeToolMenu();
}

void MainWindowToolbarMenu::freeToolMenu() {
    for (MenuSelectToolbarData* data: toolbarMenuData) { delete data; }
    this->toolbarMenuData.clear();
}

void MainWindowToolbarMenu::setTmpDisabled(bool disabled) {
    for (MenuSelectToolbarData* data: this->toolbarMenuData) { gtk_widget_set_sensitive(data->item, !disabled); }
}

void MainWindowToolbarMenu::selectToolbar(Settings* settings, ToolMenuHandler* toolbar) {
    selectedToolbar = nullptr;

    const std::string& selectedId = settings->getSelectedToolbar();

    for (ToolbarData* d: *toolbar->getModel()->getToolbars()) {
        if (selectedToolbar == nullptr) {
            selectedToolbar = d;
        }

        if (selectedId == d->getId()) {
            selectedToolbar = d;
            break;
        }
    }
}

auto MainWindowToolbarMenu::getSelectedToolbar() -> ToolbarData* { return selectedToolbar; }

void MainWindowToolbarMenu::removeOldElements(GtkMenuShell* menubar) {
    for (GtkWidget* w: menuitems) { gtk_container_remove(GTK_CONTAINER(menubar), w); }
    menuitems.clear();

    freeToolMenu();
}

void MainWindowToolbarMenu::tbSelectMenuitemActivated(GtkCheckMenuItem* checkmenuitem, MenuSelectToolbarData* data) {
    data->tbm->menuClicked(checkmenuitem, data);
}

void MainWindowToolbarMenu::menuClicked(GtkCheckMenuItem* menuitem, MenuSelectToolbarData* data) {
    if (!gtk_check_menu_item_get_active(menuitem)) {
        // Ignore disabled menus
        return;
    }

    win->toolbarSelected(data->d);

    for (int i = 0; i < static_cast<int>(this->toolbarMenuData.size()); i++) {
        if (data->index == i) {
            continue;
        }

        GtkWidget* w = this->toolbarMenuData[i]->item;

        if (GTK_IS_CHECK_MENU_ITEM(w)) {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), false);
        }
    }
}

void MainWindowToolbarMenu::addToolbarMenuEntry(ToolbarData* d, GtkMenuShell* menubar, int& menuPos) {
    GtkWidget* item = gtk_check_menu_item_new_with_label(d->getName().c_str());
    gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), true);
    gtk_widget_show(item);
    gtk_menu_shell_insert(menubar, item, menuPos);

    menuitems.push_back(item);

    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), d == selectedToolbar);

    auto* data = new MenuSelectToolbarData(this, item, d, toolbarMenuData.size());
    toolbarMenuData.push_back(data);

    g_signal_connect(item, "toggled", G_CALLBACK(tbSelectMenuitemActivated), data);

    if (inPredefinedSection && !d->isPredefined()) {
        GtkWidget* separator = gtk_separator_menu_item_new();
        gtk_widget_show(separator);
        gtk_menu_shell_insert(menubar, separator, menuPos++);

        inPredefinedSection = false;
        menuitems.push_back(separator);
    }
}

void MainWindowToolbarMenu::updateToolbarMenu(GtkMenuShell* menubar, Settings* settings, ToolMenuHandler* toolbar) {
    selectToolbar(settings, toolbar);
    removeOldElements(menubar);

    inPredefinedSection = true;

    int menuPos = 0;
    for (ToolbarData* d: *toolbar->getModel()->getToolbars()) {
        addToolbarMenuEntry(d, menubar, menuPos);
        menuPos++;
    }
}
