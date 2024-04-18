#include "RenameLayerDialog.h"

#include <memory>  // for allocator, make_unique
#include <string>  // for string

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "control/layer/LayerController.h"  // for LayerController
#include "gui/Builder.h"
#include "undo/LayerRenameUndoAction.h"  // for LayerRenameUndoAction
#include "undo/UndoRedoHandler.h"        // for UndoRedoHandler

class GladeSearchpath;
class Layer;

constexpr auto UI_FILE = "renameLayerDialog.ui";
constexpr auto UI_DIALOG_NAME = "renameLayerDialog";

RenameLayerDialog::RenameLayerDialog(GladeSearchpath* gladeSearchPath, UndoRedoHandler* undo, LayerController* lc,
                                     Layer* l):
        lc(lc), undo(undo), l(l) {
    Builder builder(gladeSearchPath, UI_FILE);
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));
    layerNameEntry = GTK_ENTRY(builder.get("layerNameEntry"));

    gtk_editable_set_text(GTK_EDITABLE(layerNameEntry), lc->getCurrentLayerName().c_str());

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect_swapped(builder.get("btOk"), "clicked", G_CALLBACK(+[](RenameLayerDialog* self) {
                                 std::string newName = gtk_editable_get_text(GTK_EDITABLE(self->layerNameEntry));

                                 self->undo->addUndoAction(std::make_unique<LayerRenameUndoAction>(
                                         self->lc, self->l, newName, self->lc->getCurrentLayerName()));

                                 self->lc->setCurrentLayerName(newName);
                                 gtk_window_close(self->window.get());
                             }),
                             this);
}
