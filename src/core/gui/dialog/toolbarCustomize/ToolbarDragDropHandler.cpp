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

ToolbarDragDropHandler::~ToolbarDragDropHandler() = default;

void ToolbarDragDropHandler::prepareToolbarsForDragAndDrop() {
    MainWindow* win = control->getWindow();

    this->toolbars.clear();

    for (auto w: win->getToolbarWidgets()) {
        this->toolbars.emplace_back(std::make_unique<ToolbarAdapter>(w.get(), win->getToolbarName(w.get()),
                                                                     control->getWindow()->getToolMenuHandler(), win));
    }
}

void ToolbarDragDropHandler::clearToolbarsFromDragAndDrop() { this->toolbars.clear(); }

void ToolbarDragDropHandler::toolbarConfigDialogClosed() {
    this->customizeDialog.reset();

    MainWindow* win = control->getWindow();

    this->clearToolbarsFromDragAndDrop();

    auto file = Util::getConfigFile(TOOLBAR_CONFIG);
    win->getToolbarModel()->save(file);
    win->getFloatingToolbox()->hide();
}

void ToolbarDragDropHandler::configure() {
    MainWindow* win = control->getWindow();


    win->getFloatingToolbox()->showForConfiguration();

    this->prepareToolbarsForDragAndDrop();

    this->customizeDialog = std::make_unique<ToolbarCustomizeDialog>(control->getGladeSearchPath(), win, this);

    this->customizeDialog->show(GTK_WINDOW(win->getWindow()));
}
