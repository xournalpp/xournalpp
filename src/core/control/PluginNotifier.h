/*
 * Xournal++
 *
 * Notifier for plugins
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include "model/DocumentListener.h"
#include "undo/UndoRedoHandler.h"

#include "Control.h"

class UndoRedoListener;
class PluginController;

class PluginNotifier: public DocumentListener, public UndoRedoListener {
public:
    PluginNotifier(Control* control);
    ~PluginNotifier() override;

public:
    // DocumentListener interface
    void documentChanged(DocumentChangeType type) override;
    void pageSizeChanged(size_t page) override;
    void pageChanged(size_t page) override;
    void pageInserted(size_t page) override;
    void pageDeleted(size_t page) override;
    void pageSelected(size_t page) override;

public:
    // UndoRedoListener interface
    void undoRedoChanged() override;
    void undoRedoPageChanged(PageRef page) override;


private:
    Control* control;
};
