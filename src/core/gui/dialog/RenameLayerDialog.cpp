#include "RenameLayerDialog.h"

#include <memory>  // for allocator, make_unique
#include <string>  // for string

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "control/layer/LayerController.h"  // for LayerController
#include "undo/LayerRenameUndoAction.h"     // for LayerRenameUndoAction
#include "undo/UndoRedoHandler.h"           // for UndoRedoHandler

class GladeSearchpath;
class Layer;

RenameLayerDialog::RenameLayerDialog(GladeSearchpath* gladeSearchPath, UndoRedoHandler* undo, LayerController* lc,
                                     Layer* l):
        GladeGui(gladeSearchPath, "renameLayerDialog.glade", "renameLayerDialog"), lc(lc), undo(undo), l(l) {
    gtk_entry_set_text(GTK_ENTRY(get("layerNameEntry")), lc->getCurrentLayerName().c_str());

    g_signal_connect(get("renameButton"), "clicked", G_CALLBACK(renameSuccessful), this);
    g_signal_connect(get("cancelButton"), "clicked", G_CALLBACK(exitDialog), this);
}

void RenameLayerDialog::renameSuccessful(GtkButton* bttn, RenameLayerDialog* rld) {
    std::string newName = gtk_entry_get_text(GTK_ENTRY(rld->get("layerNameEntry")));

    rld->undo->addUndoAction(
            std::make_unique<LayerRenameUndoAction>(rld->lc, rld->l, newName, rld->lc->getCurrentLayerName()));

    rld->lc->setCurrentLayerName(newName);
    gtk_window_close(GTK_WINDOW(rld->window));
}

void RenameLayerDialog::exitDialog(GtkButton* bttn, RenameLayerDialog* rld) {
    gtk_window_close(GTK_WINDOW(rld->window));
}

RenameLayerDialog::~RenameLayerDialog() = default;

void RenameLayerDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    gtk_dialog_run(GTK_DIALOG(this->window));
    gtk_widget_hide(this->window);
}
