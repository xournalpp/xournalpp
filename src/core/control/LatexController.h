/*
 * Xournal++
 *
 * Controller for Latex stuff
 *
 * @author W Brenna
 * http://wbrenna.ca
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr
#include <string>  // for string

#include <gio/gio.h>  // for GAsyncResult, GCancellable
#include <gtk/gtk.h>  // for GtkTextBuffer
#include <poppler.h>  // for GObject

#include "control/latex/LatexGenerator.h"  // for LatexGenerator
#include "gui/dialog/LatexDialog.h"        // for LatexDialog
#include "model/PageRef.h"                 // for PageRef

#include "filesystem.h"  // for path

class Control;
class TexImage;
class Document;
class XojPageView;
class Layer;
class Element;
class LatexSettings;

class LatexController {
public:
    LatexController() = delete;
    LatexController(const LatexController& other) = delete;
    LatexController& operator=(const LatexController& other) = delete;
    LatexController(Control* control);
    virtual ~LatexController();

public:
    /**
     * Open a LatexDialog, wait for the user to provide the LaTeX formula, and
     * insert the rendered formula into the document if the supplied LaTeX is
     * valid.
     */
    void run();

private:
    /**
     * Provides information about whether a particular dependency was found or not.
     */
    class FindDependencyStatus {
    public:
        FindDependencyStatus(bool success, std::string errorMsg): success(success), errorMsg(errorMsg){};
        bool success;
        std::string errorMsg;
    };

    /**
     * Set the required LaTeX files, returning false if at least one of them is not found.
     */
    LatexController::FindDependencyStatus findTexDependencies();

    /**
     * Find a selected tex element, and load it
     */
    void findSelectedTexElement();

    /**
     * If a previous image/text is selected, delete it
     */
    void deleteOldImage();

    /**
     * Asynchronously runs the LaTeX command and then updates the TeX image with
     * the given LaTeX string. If the preview is already being updated, then
     * this method will be a no-op.
     */
    void triggerImageUpdate(const std::string& texString);

    /**
     * Show the LaTex Editor dialog, returning the final formula input by the
     * user. If the input was cancelled, the resulting string will be the same
     * as the initial formula.
     */
    std::string showTexEditDialog();

    /**
     * Signal handler, updates the rendered image when the text in the editor
     * changes.
     */
    static void handleTexChanged(GtkTextBuffer* buffer, LatexController* self);

    /**
     * Updates the display once the PDF file is generated.
     *
     * If the Latex text has changed since the last update, triggerPreviewUpdate
     * will be called again.
     */
    static void onPdfRenderComplete(GObject* procObj, GAsyncResult* res, LatexController* self);

    void updateStatus();
    bool isUpdating();

    /**
     * Load the preview PDF from disk and create a TexImage object.
     */
    std::unique_ptr<TexImage> loadRendered(std::string renderedTex);

    /**
     * Insert the generated preview TexImage into the current page.
     */
    void insertTexImage();

private:
    Control* control = nullptr;
    LatexSettings const& settings;

    /**
     * The contents of the latex template, loaded from disk.
     */
    std::string latexTemplate;

    /**
     * LaTex editor dialog
     */
    LatexDialog dlg;

    /**
     * Tex binary full path
     */
    fs::path pdflatexPath;

    /**
     * The original TeX string when the dialog was opened, or the empty string
     * if creating a new LaTeX element.
     */
    std::string initialTex;

    /**
     * The last TeX string shown in the preview.
     */
    std::string lastPreviewedTex;

    /**
     * Whether a preview is currently being generated.
     */
    GCancellable* updating_cancellable = nullptr;

    /**
     * The output of the last run of the
     * TeX command.
     */
    std::string texProcessOutput;

    /**
     * Whether the current TeX string is valid.
     */
    bool isValidTex = false;

    /**
     * X-Position
     */
    int posx = 0;

    /**
     * Y-Position
     */
    int posy = 0;

    /**
     * Image width
     */
    int imgwidth = 0;

    /**
     * Image height
     */
    int imgheight = 0;

    /**
     * Document
     */
    Document* doc = nullptr;

    /**
     * Page View
     */
    XojPageView* view = nullptr;

    /**
     * Selected Page
     */
    PageRef page;

    /**
     * Selected layer
     */
    Layer* layer = nullptr;

    /**
     * The directory in which the LaTeX files will be generated. Note that this
     * should be within a system temporary directory.
     */
    fs::path texTmpDir;

    /**
     * The element that is currently being edited.
     */
    Element* selectedElem = nullptr;

    /**
     * The controller owns the rendered preview in order to be able to delete it
     * when a new render is created
     */
    std::unique_ptr<TexImage> temporaryRender;

    LatexGenerator generator;
};
