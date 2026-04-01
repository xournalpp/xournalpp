/*
 * Xournal++
 *
 * Latex dialog for the internal editor
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <cairo.h>               // for cairo_t
#include <gtk/gtk.h>             // for GtkWidget, GtkTextBuffer, GtkWindow
#include <gtk/gtkcssprovider.h>  // for GtkCssProvider
#include <poppler.h>             // for PopplerDocument, PopplerPage

#include "util/raii/GObjectSPtr.h"

#include "AbstractLatexDialog.h"

class LatexController;
class LatexSettings;
class GladeSearchpath;

class IntEdLatexDialog final: public AbstractLatexDialog {
public:
    IntEdLatexDialog() = delete;
    IntEdLatexDialog(const IntEdLatexDialog& other) = delete;
    IntEdLatexDialog& operator=(const IntEdLatexDialog& other) = delete;
    IntEdLatexDialog(GladeSearchpath* gladeSearchPath, std::unique_ptr<LatexController> texCtrl);
    virtual ~IntEdLatexDialog();

public:
    // Necessary for the controller in order to connect the 'text-changed'
    // signal handler
    GtkTextBuffer* getTextBuffer();
    std::string getBufferContents() override;

private:
    // Text field
    xoj::util::GObjectSPtr<GtkCssProvider> cssProvider;  ///< For tex code display font and size
    GtkWidget* texBox;
    GtkTextBuffer* textBuffer;

    /**
     * The final LaTeX string to save once the dialog is closed.
     */
    std::string finalLatex;

    std::function<void()> callback;
};
