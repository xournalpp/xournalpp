#include "ToolbarDragDropHandler.h"

#include <string>  // for allocator, string

#include <gtk/gtk.h>  // for GtkWidget, GTK_TO...

#include "control/Control.h"      // for Control
#include "gui/FloatingToolbox.h"  // for FloatingToolbox
#include "gui/MainWindow.h"       // for MainWindow
#include "gui/toolbarMenubar/model/ToolbarData.h"
#include "gui/toolbarMenubar/model/ToolbarEntry.h"
#include "gui/toolbarMenubar/model/ToolbarModel.h"  // for ToolbarModel
#include "gui/widgets/ToolbarBox.h"
#include "util/PathUtil.h"  // for getConfigFile
#include "util/PopupWindowWrapper.h"

#include "ToolbarCustomizeDialog.h"  // for ToolbarCustomizeD...
#include "config-dev.h"              // for TOOLBAR_CONFIG

ToolbarDragDropHandler::ToolbarDragDropHandler(Control* control): control(control) {}

ToolbarDragDropHandler::~ToolbarDragDropHandler() = default;

void ToolbarDragDropHandler::prepareToolbarsForDragAndDrop() {
    MainWindow* win = control->getWindow();
    ToolMenuHandler* th = win->getToolMenuHandler();

    for (const auto& tb: win->getToolbars()) {
        tb->startEditing(th);
    }
}

void ToolbarDragDropHandler::toolbarConfigDialogClosed() {
    MainWindow* win = control->getWindow();
    ToolbarData* data = win->getSelectedToolbar();
    for (const auto& tb: win->getToolbars()) {
        data->setEntry(tb->endEditing());
    }

    auto file = Util::getConfigFile(TOOLBAR_CONFIG);
    win->getToolbarModel()->save(file);
    win->getFloatingToolbox()->hide();
}

void ToolbarDragDropHandler::configure() {
    MainWindow* win = control->getWindow();

    win->getFloatingToolbox()->showForConfiguration();

    this->prepareToolbarsForDragAndDrop();

    xoj::popup::PopupWindowWrapper<ToolbarCustomizeDialog> dlg(control->getGladeSearchPath(), win, this);
    dlg.show(control->getGtkWindow(), /* modal */ false);
}
