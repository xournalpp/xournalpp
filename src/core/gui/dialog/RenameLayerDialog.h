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

#include <gtk/gtk.h>

#include "util/raii/GtkWindowUPtr.h"

class GladeSearchpath;
class Layer;
class LayerController;
class UndoRedoHandler;

class RenameLayerDialog {
public:
    RenameLayerDialog(GladeSearchpath* gladeSearchPath, UndoRedoHandler* undo, LayerController* lc, Layer* l);

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    LayerController* lc;
    UndoRedoHandler* undo;
    Layer* l;

    xoj::util::GtkWindowUPtr window;
    GtkEntry* layerNameEntry;
};
