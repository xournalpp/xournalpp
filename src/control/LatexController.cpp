#include "LatexController.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <memory>
#include <regex>
#include <sstream>
#include <utility>

#include "gui/XournalView.h"
#include "gui/dialog/LatexDialog.h"
#include "latex/LatexGenerator.h"
#include "undo/InsertUndoAction.h"

#include "Control.h"
#include "StringUtils.h"
#include "Util.h"
#include "XojMsgBox.h"
#include "i18n.h"
#include "pixbuf-utils.h"

LatexController::LatexController(Control* control):
        control(control),
        settings(control->getSettings()->latexSettings),
        dlg(control->getGladeSearchPath()),
        doc(control->getDocument()),
        texTmpDir(Util::getTmpDirSubfolder("tex")),
        generator(settings) {
    Util::ensureFolderExists(this->texTmpDir);
}

LatexController::~LatexController() { this->control = nullptr; }

/**
 * Find the tex executable, return false if not found
 */
auto LatexController::findTexDependencies() -> LatexController::FindDependencyStatus {
    std::string templatePathName = this->settings.globalTemplatePath.string();
    if (fs::is_regular_file(fs::status(templatePathName))) {
        std::ifstream is(templatePathName, std::ios_base::binary);
        if (!is.is_open()) {
            g_message("%s", templatePathName.c_str());
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
    int pageNr = this->control->getCurrentPageNo();
    if (pageNr == -1) {
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
        this->posx = theSelection->getXOnView();
        this->posy = theSelection->getYOnView();

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

        if (layout->getViewAt(centerX, centerY) == this->view) {
            // Pick the center of the visible area (converting from screen to page coordinates)
            this->posx = (centerX - this->view->getX()) / zoom;
            this->posy = (centerY - this->view->getY()) / zoom;
        } else {
            // No better location, so just center it on the page (possibly out of viewport)
            this->posx = 0.5 * this->page->getWidth();
            this->posy = 0.5 * this->page->getHeight();
        }
    }
    this->doc->unlock();
}

auto LatexController::showTexEditDialog() -> string {
    // Attach the signal handler before setting the buffer text so that the
    // callback is triggered
    gulong signalHandler = g_signal_connect(dlg.getTextBuffer(), "changed", G_CALLBACK(handleTexChanged), this);
    bool isNewFormula = this->initialTex.empty();
    this->dlg.setFinalTex(isNewFormula ? "x^2" : this->initialTex);

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
    if (this->isUpdating) {
        return;
    }

    this->setUpdating(true);
    this->lastPreviewedTex = texString;
    const std::string texContents = LatexGenerator::templateSub(
            texString, this->latexTemplate, this->control->getToolHandler()->getTool(TOOL_TEXT).getColor());
    auto result = generator.asyncRun(this->texTmpDir, texContents);
    if (auto* err = std::get_if<LatexGenerator::GenError>(&result)) {
        this->setUpdating(false);
        XojMsgBox::showErrorToUser(this->control->getGtkWindow(), err->message);
    } else if (auto** proc = std::get_if<GSubprocess*>(&result)) {
        g_subprocess_wait_check_async(*proc, nullptr, reinterpret_cast<GAsyncReadyCallback>(onPdfRenderComplete), this);
    }
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
    g_assert(self->isUpdating);
    GError* err = nullptr;
    GSubprocess* proc = G_SUBPROCESS(procObj);
    g_subprocess_wait_check_finish(proc, res, &err);

    const string currentTex = self->dlg.getBufferContents();
    bool shouldUpdate = self->lastPreviewedTex != currentTex;
    if (err != nullptr) {
        self->isValidTex = false;
        if (!g_error_matches(err, G_SPAWN_EXIT_ERROR, 1)) {
            // The error was not caused by invalid LaTeX.
            string message =
                    FS(_F("Latex generation encountered an error: {1} (exit code: {2})") % err->message % err->code);
            g_warning("latex: %s", message.c_str());
            XojMsgBox::showErrorToUser(self->control->getGtkWindow(), message);
        }
        fs::path pdfPath = self->texTmpDir / "tex.pdf";
        fs::remove(pdfPath);
        g_error_free(err);
    } else {
        self->isValidTex = true;
        self->temporaryRender = self->loadRendered(currentTex);
        if (self->temporaryRender != nullptr) {
            self->dlg.setTempRender(self->temporaryRender->getPdf());
        }
    }

    self->setUpdating(false);
    if (shouldUpdate) {
        self->triggerImageUpdate(currentTex);
    }
}

void LatexController::setUpdating(bool newValue) {
    GtkWidget* okButton = this->dlg.get("texokbutton");
    bool buttonEnabled = true;
    if ((!this->isUpdating && newValue) || (this->isUpdating && !newValue)) {
        // Disable LatexDialog OK button while updating. This is a workaround
        // for the fact that 1) the LatexController only lives while the dialog
        // is open; 2) the preview is generated asynchronously; and 3) the `run`
        // method that inserts the TexImage object is called synchronously after
        // the dialog is closed with the OK button.
        buttonEnabled = !newValue;
    }


    // Invalid LaTeX will generate an invalid PDF, so disable the OK button if
    // needed.
    buttonEnabled = buttonEnabled && this->isValidTex;

    gtk_widget_set_sensitive(okButton, buttonEnabled);

    GtkLabel* errorLabel = GTK_LABEL(this->dlg.get("texErrorLabel"));
    gtk_label_set_text(errorLabel, this->isValidTex ? "" : N_("The formula is empty when rendered or invalid."));

    this->isUpdating = newValue;
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
