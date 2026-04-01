/*
 * Xournal++
 *
 * Latex dialog for an external editor
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <glib.h>     // for gpointer
#include <gtk/gtk.h>  // for GtkWidget

#include "gui/GladeSearchpath.h"
#include "gui/dialog/AbstractLatexDialog.h"

class LatexController;

class ExtEdLatexDialog final: public AbstractLatexDialog {
public:
    ExtEdLatexDialog() = delete;
    ExtEdLatexDialog(const ExtEdLatexDialog& other) = delete;
    ExtEdLatexDialog& operator=(const ExtEdLatexDialog& other) = delete;
    ExtEdLatexDialog(GladeSearchpath* gladeSearchPath, std::unique_ptr<LatexController> texCtrl);
    ~ExtEdLatexDialog() override;

    std::string getBufferContents() override;

private:
    void setCompilationStatus(bool isTexValid, bool isCompilationDone, const std::string& compilationOutput) override;

    /**
     * @brief a callback for the editor exiting
     */
    static void editorWaitCallback(GObject* processObj, GAsyncResult* res, ExtEdLatexDialog* self);

    /**
     * @brief Open the editor or do nothing if it's already open.
     */
    void openEditor();

private:
    GtkButton* btContEdit;
    fs::path tempf;
    GCancellable* editorCancellable = nullptr;
};
