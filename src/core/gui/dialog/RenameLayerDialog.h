/*
 * Xournal++
 *
 * The rename layer dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkButton, GtkWindow

#include "gui/GladeGui.h"  // for GladeGui

class GladeSearchpath;
class Layer;
class LayerController;
class UndoRedoHandler;

class RenameLayerDialog: public GladeGui {
public:
    RenameLayerDialog(GladeSearchpath* gladeSearchPath, UndoRedoHandler* undo, LayerController* lc, Layer* l);
    ~RenameLayerDialog() override;

public:
    void show(GtkWindow* parent) override;

private:
    static void exitDialog(GtkButton* bttn, RenameLayerDialog* rld);
    static void renameSuccessful(GtkButton* bttn, RenameLayerDialog* rld);

private:
    LayerController* lc;
    UndoRedoHandler* undo;
    Layer* l;
};
