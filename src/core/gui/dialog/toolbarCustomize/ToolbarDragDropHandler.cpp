#include "ToolbarDragDropHandler.h"

#include <string>  // for allocator, string

#include <gtk/gtk.h>  // for GtkWidget, GTK_TO...

#include "control/Control.h"                        // for Control
#include "gui/FloatingToolbox.h"                    // for FloatingToolbox
#include "gui/MainWindow.h"                         // for MainWindow
#include "gui/toolbarMenubar/model/ToolbarModel.h"  // for ToolbarModel
#include "util/PathUtil.h"                          // for getConfigFile

#include "ToolbarAdapter.h"          // for ToolbarAdapter
#include "ToolbarCustomizeDialog.h"  // for ToolbarCustomizeD...
#include "config-dev.h"              // for TOOLBAR_CONFIG

ToolbarDragDropHandler::ToolbarDragDropHandler(Control* control): control(control) {}

ToolbarDragDropHandler::~ToolbarDragDropHandler() { clearToolbarsFromDragAndDrop(); }

void ToolbarDragDropHandler::prepareToolbarsForDragAndDrop() {
    int len = 0;
    MainWindow* win = control->getWindow();
    GtkWidget** widgets = win->getToolbarWidgets(len);

    this->toolbars = new ToolbarAdapter*[len + 1];
    this->toolbars[len] = nullptr;

    for (int i = 0; i < len; i++) {
        GtkWidget* w = widgets[i];
        this->toolbars[i] = new ToolbarAdapter(w, win->getToolbarName(GTK_TOOLBAR(w)),
                                               control->getWindow()->getToolMenuHandler(), control->getWindow());
    }
}

void ToolbarDragDropHandler::clearToolbarsFromDragAndDrop() {
    if (this->toolbars == nullptr) {
        return;
    }

    for (int i = 0; this->toolbars[i]; i++) { delete this->toolbars[i]; }
    delete[] this->toolbars;

    this->toolbars = nullptr;
}

void ToolbarDragDropHandler::toolbarConfigDialogClosed() {
    delete this->customizeDialog;
    this->customizeDialog = nullptr;

    MainWindow* win = control->getWindow();

    this->clearToolbarsFromDragAndDrop();

    auto file = Util::getConfigFile(TOOLBAR_CONFIG);
    win->getToolbarModel()->save(file);
    win->floatingToolbox->hide();
}

void ToolbarDragDropHandler::configure() {
    MainWindow* win = control->getWindow();


    win->floatingToolbox->showForConfiguration();

    this->prepareToolbarsForDragAndDrop();

    this->customizeDialog = new ToolbarCustomizeDialog(control->getGladeSearchPath(), win, this);

    this->customizeDialog->show(GTK_WINDOW(win->getWindow()));
}

auto ToolbarDragDropHandler::isInDragAndDrop() -> bool { return this->toolbars != nullptr; }
