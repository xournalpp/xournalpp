#include "LatexController.h"

#include <cstdlib>   // for free
#include <fstream>   // for ifstream, basic_istream
#include <iterator>  // for istreambuf_iterator, ope...
#include <memory>    // for unique_ptr, allocator
#include <optional>  // for optional
#include <utility>   // for move
#include <variant>   // for get_if

#include <glib.h>  // for g_error_free, g_error_ma...

#include "control/Tool.h"                    // for Tool
#include "control/ToolEnums.h"               // for TOOL_TEXT
#include "control/ToolHandler.h"             // for ToolHandler
#include "control/latex/LatexGenerator.h"    // for LatexGenerator::GenError
#include "control/settings/LatexSettings.h"  // for LatexSettings
#include "control/settings/Settings.h"       // for Settings
#include "control/tools/EditSelection.h"     // for EditSelection
#include "gui/Layout.h"                      // for Layout
#include "gui/MainWindow.h"                  // for MainWindow
#include "gui/PageView.h"                    // for XojPageView
#include "gui/XournalView.h"                 // for XournalView
#include "gui/dialog/LatexDialog.h"          // for LatexDialog
#include "model/Document.h"                  // for Document
#include "model/Element.h"                   // for Element
#include "model/Layer.h"                     // for Layer
#include "model/TexImage.h"                  // for TexImage
#include "model/Text.h"                      // for Text
#include "model/XojPage.h"                   // for XojPage
#include "undo/InsertUndoAction.h"           // for InsertUndoAction
#include "undo/UndoRedoHandler.h"            // for UndoRedoHandler
#include "util/Color.h"                      // for Color, get_color_contrast
#include "util/PathUtil.h"                   // for ensureFolderExists, getT...
#include "util/PlaceholderString.h"          // for PlaceholderString
#include "util/Rectangle.h"                  // for Rectangle
#include "util/Util.h"                       // for npos
#include "util/XojMsgBox.h"                  // for XojMsgBox
#include "util/i18n.h"                       // for FS, _, _F, N_

#include "Control.h"  // for Control

using std::string;

constexpr Color LIGHT_PREVIEW_BACKGROUND = Colors::white;
constexpr Color DARK_PREVIEW_BACKGROUND = Colors::black;

LatexController::LatexController(Control* control):
        control(control),
        settings(control->getSettings()->latexSettings),
        dlg(control->getGladeSearchPath(), settings),
        doc(control->getDocument()),
        texTmpDir(Util::getTmpDirSubfolder("tex")),
        generator(settings) {
    Util::ensureFolderExists(this->texTmpDir);
}

LatexController::~LatexController() {
    if (updating_cancellable) {
        g_cancellable_cancel(updating_cancellable);
        g_object_unref(updating_cancellable);
    }

    this->control = nullptr;
}

/**
 * Find the tex executable, return false if not found
 */
auto LatexController::findTexDependencies() -> LatexController::FindDependencyStatus {
    auto templatePath = this->settings.globalTemplatePath;
    if (fs::is_regular_file(templatePath)) {
        std::ifstream is(templatePath, std::ios_base::binary);
        if (!is.is_open()) {
            g_message("%s", templatePath.string().c_str());
            string msg = _("Global template file does not exist. Please check your settings.");
            return LatexController::FindDependencyStatus(false, msg);
        }
        this->latexTemplate = std::string(std::istreambuf_iterator<char>(is), {});
        if (!is.good()) {
            string msg = _("Failed to read global template file. Please check your settings.");
            return LatexController::FindDependencyStatus(false, msg);
        }

        return LatexController::FindDependencyStatus(true, "");
    } else {
        string msg = _("Global template file is not a regular file. Please check your settings. ");
        return LatexController::FindDependencyStatus(false, msg);
    }
}

/**
 * Find a selected tex element, and load it
 */
void LatexController::findSelectedTexElement() {
    this->doc->lock();
    auto pageNr = this->control->getCurrentPageNo();
    if (pageNr == npos) {
        this->doc->unlock();
        return;
    }
    this->view = this->control->getWindow()->getXournal()->getViewFor(pageNr);
    if (view == nullptr) {
        this->doc->unlock();
        return;
    }

    // we get the selection
    this->page = this->doc->getPage(pageNr);
    this->layer = page->getSelectedLayer();

    this->selectedElem = view->getSelectedTex() != nullptr ? static_cast<Element*>(view->getSelectedTex()) :
                                                             static_cast<Element*>(view->getSelectedText());
    if (this->selectedElem) {
        // this will get the position of the Latex properly
        EditSelection* theSelection = control->getWindow()->getXournal()->getSelection();
        xoj::util::Rectangle<double> rect = theSelection->getSnappedBounds();
        this->posx = rect.x;
        this->posy = rect.y;

        if (auto* img = dynamic_cast<TexImage*>(this->selectedElem)) {
            this->initialTex = img->getText();
        } else if (auto* txt = dynamic_cast<Text*>(this->selectedElem)) {
            this->initialTex = "\\text{" + txt->getText() + "}";
        }
        this->imgwidth = this->selectedElem->getElementWidth();
        this->imgheight = this->selectedElem->getElementHeight();
    } else {
        // This is a new latex object, so here we pick a convenient initial location
        const double zoom = this->control->getWindow()->getXournal()->getZoom();
        Layout* const layout = this->control->getWindow()->getLayout();

        // Calculate coordinates (screen) of the center of the visible area
        const auto visibleBounds = layout->getVisibleRect();
        const double centerX = visibleBounds.x + 0.5 * visibleBounds.width;
        const double centerY = visibleBounds.y + 0.5 * visibleBounds.height;

        if (layout->getPageViewAt(static_cast<int>(std::round(centerX)), static_cast<int>(std::round(centerY))) ==
            this->view) {
            // Pick the center of the visible area (converting from screen to page coordinates)
            this->posx = (centerX - this->view->getX()) / zoom;
            this->posy = (centerY - this->view->getY()) / zoom;
        } else {
            // No better location, so just center it on the page (possibly out of viewport)
            this->posx = this->page->getWidth() / 2;
            this->posy = this->page->getHeight() / 2;
        }
    }
    this->doc->unlock();
}

auto LatexController::showTexEditDialog() -> string {
    // Attach the signal handler before setting the buffer text so that the
    // callback is triggered
    gulong signalHandler = g_signal_connect(dlg.getTextBuffer(), "changed", G_CALLBACK(handleTexChanged), this);
    bool isNewFormula = this->initialTex.empty();
    this->dlg.setFinalTex(isNewFormula ? this->settings.defaultText : this->initialTex);

    if (this->temporaryRender != nullptr) {
        this->dlg.setTempRender(this->temporaryRender->getPdf());
    }

    this->dlg.show(GTK_WINDOW(control->getWindow()->getWindow()), isNewFormula);
    g_signal_handler_disconnect(dlg.getTextBuffer(), signalHandler);

    string result = this->dlg.getFinalTex();
    // If the user cancelled, there is no change in the latex string.
    result = result.empty() ? initialTex : result;
    return result;
}

void LatexController::triggerImageUpdate(const string& texString) {
    if (this->isUpdating()) {
        return;
    }

    Color textColor = this->control->getToolHandler()->getTool(TOOL_TEXT).getColor();

    // Determine a background color that has enough contrast with the text color:
    if (Util::get_color_contrast(textColor, LIGHT_PREVIEW_BACKGROUND) > 0.5) {
        this->dlg.setPreviewBackgroundColor(LIGHT_PREVIEW_BACKGROUND);
    } else {
        this->dlg.setPreviewBackgroundColor(DARK_PREVIEW_BACKGROUND);
    }

    this->lastPreviewedTex = texString;
    const std::string texContents = LatexGenerator::templateSub(texString, this->latexTemplate, textColor);
    auto result = generator.asyncRun(this->texTmpDir, texContents);
    if (auto* err = std::get_if<LatexGenerator::GenError>(&result)) {
        XojMsgBox::showErrorToUser(this->control->getGtkWindow(), err->message);
    } else if (auto** proc = std::get_if<GSubprocess*>(&result)) {
        // Render the TeX and capture the process' output.
        updating_cancellable = g_cancellable_new();
        char* stdinBuff = nullptr;  // No stdin

        g_subprocess_communicate_utf8_async(*proc, stdinBuff, updating_cancellable,
                                            reinterpret_cast<GAsyncReadyCallback>(onPdfRenderComplete), this);
    }

    updateStatus();
}

/**
 * Text-changed handler: when the Buffer in the dialog changes, this handler
 * removes the previous existing render and creates a new one. We need to do it
 * through 'self' because signal handlers cannot directly access non-static
 * methods and non-static fields such as 'dlg' so we need to wrap all the dlg
 * method inside small methods in 'self'. To improve performance, we render the
 * text asynchronously.
 */
void LatexController::handleTexChanged(GtkTextBuffer* buffer, LatexController* self) {
    self->triggerImageUpdate(self->dlg.getBufferContents());
}

void LatexController::onPdfRenderComplete(GObject* procObj, GAsyncResult* res, LatexController* self) {
    GError* err = nullptr;
    bool procExited = false;
    GSubprocess* proc = G_SUBPROCESS(procObj);

    // Extract the process' output and store it.
    {
        char* procStdout_ptr = nullptr;

        // Stdout and stderr should be merged.
        procExited = g_subprocess_communicate_utf8_finish(proc, res, &procStdout_ptr, nullptr, &err);

        // If we have stdout, store it.
        if (procStdout_ptr != nullptr) {
            self->texProcessOutput = procStdout_ptr;
            free(procStdout_ptr);
        } else {
            g_warning("latex command: no stdout stream");
        }
    }

    if (err != nullptr) {
        if (g_error_matches(err, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            // the render was canceled
            g_error_free(err);
            return;
        } else if (!g_error_matches(err, G_SPAWN_EXIT_ERROR, 1)) {
            // The error was not caused by invalid LaTeX.
            string message =
                    FS(_F("Latex generation encountered an error: {1} (exit code: {2})") % err->message % err->code);
            g_warning("latex: %s", message.c_str());
            XojMsgBox::showErrorToUser(self->control->getGtkWindow(), message);
        }

        self->isValidTex = false;
        g_error_free(err);
    } else if (procExited && g_subprocess_get_exit_status(proc) != 0) {
        // Command exited with non-zero exit status.

        self->isValidTex = false;
    } else {
        self->isValidTex = true;
    }

    // Delete the PDF if the TeX is invalid.
    if (!self->isValidTex) {
        fs::path pdfPath = self->texTmpDir / "tex.pdf";
        fs::remove(pdfPath);
    }

    const string currentTex = self->dlg.getBufferContents();
    bool shouldUpdate = self->lastPreviewedTex != currentTex;
    if (self->isValidTex) {
        self->temporaryRender = self->loadRendered(currentTex);
        if (self->temporaryRender != nullptr) {
            self->dlg.setTempRender(self->temporaryRender->getPdf());
        }
    }

    g_clear_object(&self->updating_cancellable);
    g_clear_object(&proc);

    self->updateStatus();
    if (shouldUpdate) {
        self->triggerImageUpdate(currentTex);
    }
}

bool LatexController::isUpdating() { return updating_cancellable; }

void LatexController::updateStatus() {
    GtkWidget* okButton = this->dlg.get("texokbutton");
    bool buttonEnabled = true;
    // Disable LatexDialog OK button while updating. This is a workaround
    // for the fact that 1) the LatexController only lives while the dialog
    // is open; 2) the preview is generated asynchronously; and 3) the `run`
    // method that inserts the TexImage object is called synchronously after
    // the dialog is closed with the OK button.
    buttonEnabled = !this->isUpdating();

    // Invalid LaTeX will generate an invalid PDF, so disable the OK button if
    // needed.
    buttonEnabled = buttonEnabled && this->isValidTex;

    gtk_widget_set_sensitive(okButton, buttonEnabled);

    // Show error warning only if LaTeX is invalid.
    GtkLabel* errorLabel = GTK_LABEL(this->dlg.get("texErrorLabel"));
    gtk_label_set_text(errorLabel, this->isValidTex ? "" : N_("The formula is empty when rendered or invalid."));

    // Update the output pane.
    GtkTextView* commandOutputDisplay = GTK_TEXT_VIEW(this->dlg.get("texCommandOutputText"));
    GtkTextBuffer* commandOutputBuffer = gtk_text_view_get_buffer(commandOutputDisplay);
    gtk_text_buffer_set_text(commandOutputBuffer, this->texProcessOutput.c_str(), -1);
}

void LatexController::deleteOldImage() {
    if (this->selectedElem) {
        EditSelection selection(control->getUndoRedoHandler(), selectedElem, view, page);
        this->view->getXournal()->deleteSelection(&selection);
        this->selectedElem = nullptr;
    }
}

auto LatexController::loadRendered(string renderedTex) -> std::unique_ptr<TexImage> {
    if (!this->isValidTex) {
        return nullptr;
    }

    fs::path pdfPath = texTmpDir / "tex.pdf";
    auto contents = Util::readString(pdfPath, true);
    if (!contents) {
        return nullptr;
    }

    auto img = std::make_unique<TexImage>();
    GError* err{};
    bool loaded = img->loadData(std::move(*contents), &err);

    if (err != nullptr) {
        string message = FS(_F("Could not load LaTeX PDF file: {1}") % err->message);
        g_message("%s", message.c_str());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), message);
        g_error_free(err);
        return nullptr;
    } else if (!loaded || !img->getPdf()) {
        XojMsgBox::showErrorToUser(control->getGtkWindow(), FS(_F("Could not load LaTeX PDF file")));
        return nullptr;
    }

    img->setX(posx);
    img->setY(posy);
    img->setText(std::move(renderedTex));
    if (imgheight) {
        double ratio = img->getElementWidth() / img->getElementHeight();
        if (ratio == 0) {
            img->setWidth(imgwidth == 0 ? 10 : imgwidth);
        } else {
            img->setWidth(imgheight * ratio);
        }
        img->setHeight(imgheight);
    }

    return img;
}

void LatexController::insertTexImage() {
    g_assert(this->temporaryRender != nullptr);
    TexImage* img = this->temporaryRender.release();

    this->control->clearSelectionEndText();
    this->deleteOldImage();

    doc->lock();
    layer->addElement(img);
    view->rerenderElement(img);
    doc->unlock();
    control->getUndoRedoHandler()->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, img));

    // Select element
    auto* selection = new EditSelection(control->getUndoRedoHandler(), img, view, page);
    view->getXournal()->setSelection(selection);
}

void LatexController::run() {
    auto depStatus = this->findTexDependencies();
    if (!depStatus.success) {
        XojMsgBox::showErrorToUser(control->getGtkWindow(), depStatus.errorMsg);
        return;
    }

    this->findSelectedTexElement();
    const string newTex = this->showTexEditDialog();

    if (this->initialTex != newTex) {
        g_assert(this->isValidTex);
        this->insertTexImage();
    }
}
