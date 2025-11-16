#include "LatexController.h"

#include <cstdlib>   // for free
#include <fstream>   // for ifstream, basic_istream
#include <iterator>  // for istreambuf_iterator, ope...
#include <limits>    // for numeric_limits
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
#include "gui/dialog/ExtEdLatexDialog.h"     // for ExtEdLatexDialog
#include "gui/dialog/IntEdLatexDialog.h"     // for IntEdLatexDialog
#include "model/Document.h"                  // for Document
#include "model/Element.h"                   // for Element
#include "model/Layer.h"                     // for Layer
#include "model/TexImage.h"                  // for TexImage
#include "model/Text.h"                      // for Text
#include "model/XojPage.h"                   // for XojPage
#include "undo/InsertUndoAction.h"           // for InsertUndoAction
#include "undo/UndoRedoHandler.h"            // for UndoRedoHandler
#include "util/Assert.h"                     // for xoj_assert
#include "util/Color.h"                      // for Color, get_color_contrast
#include "util/PathUtil.h"                   // for ensureFolderExists, getT...
#include "util/PlaceholderString.h"          // for PlaceholderString
#include "util/PopupWindowWrapper.h"         // for PopupWindowWrapper
#include "util/Rectangle.h"                  // for Rectangle
#include "util/Util.h"                       // for npos
#include "util/XojMsgBox.h"                  // for XojMsgBox
#include "util/i18n.h"                       // for FS, _, _F, N_
#include "util/safe_casts.h"                 // for round_cast

#include "Control.h"  // for Control

using std::string;

constexpr Color LIGHT_PREVIEW_BACKGROUND = Colors::white;
constexpr Color DARK_PREVIEW_BACKGROUND = Colors::black;

LatexController::LatexController(Control* control):
        control(control),
        settings(control->getSettings()->latexSettings),
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


void LatexController::showTexEditDialog(std::unique_ptr<LatexController> ctrl) {
    LatexController* texCtrl = ctrl.get();
    if (ctrl->settings.useExternalEditor) {
        xoj::popup::PopupWindowWrapper<ExtEdLatexDialog> popup(texCtrl->control->getGladeSearchPath(), std::move(ctrl));

        popup.show(GTK_WINDOW(texCtrl->control->getWindow()->getWindow()));
    } else {
        xoj::popup::PopupWindowWrapper<IntEdLatexDialog> popup(texCtrl->control->getGladeSearchPath(), std::move(ctrl));

        popup.show(GTK_WINDOW(texCtrl->control->getWindow()->getWindow()));
    }
}

void LatexController::triggerImageUpdate(const string& texString) {
    if (isUpdating()) {
        return;
    }

    Color textColor = control->getToolHandler()->getTool(TOOL_LATEX).getColor();

    // Determine a background color that has enough contrast with the text color:
    if (Util::get_color_contrast(textColor, LIGHT_PREVIEW_BACKGROUND) > 0.5) {
        dlg->setPreviewBackgroundColor(LIGHT_PREVIEW_BACKGROUND);
    } else {
        dlg->setPreviewBackgroundColor(DARK_PREVIEW_BACKGROUND);
    }

    lastPreviewedTex = texString;
    const std::string texContents = LatexGenerator::templateSub(texString, latexTemplate, textColor);
    auto result = generator.asyncRun(texTmpDir, texContents);
    if (auto* err = std::get_if<LatexGenerator::GenError>(&result)) {
        XojMsgBox::showErrorToUser(control->getGtkWindow(), err->message);
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
void LatexController::handleTexChanged(LatexController* self) {
    self->triggerImageUpdate(self->dlg->getBufferContents());
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

    const string currentTex = self->dlg->getBufferContents();
    bool shouldUpdate = self->lastPreviewedTex != currentTex;
    if (self->isValidTex) {
        self->temporaryRender = self->loadRendered(currentTex);
        if (self->temporaryRender != nullptr) {
            self->dlg->setTempRender(self->temporaryRender->getPdf());
        }
    }

    g_clear_object(&self->updating_cancellable);
    g_clear_object(&proc);

    self->updateStatus();

    // If dlg is an ExtEdLatexDialog and the user has the auto-confirm option set, the dialog might
    // have closed itself in the above updateStatus call, causing self->dlg to be nullptr.
    if (shouldUpdate && self->dlg) {
        self->triggerImageUpdate(currentTex);
    }
}

bool LatexController::isUpdating() { return updating_cancellable; }

void LatexController::updateStatus() { this->dlg->setCompilationStatus(isValidTex, !isUpdating(), texProcessOutput); }

auto LatexController::loadRendered(string renderedTex) -> std::unique_ptr<TexImage> {
    if (!this->isValidTex) {
        return nullptr;
    }

    fs::path pdfPath = texTmpDir / "tex.pdf";
    auto contents = Util::readString(pdfPath, true, std::ios::binary);
    if (!contents) {
        return nullptr;
    }

    auto img = std::make_unique<TexImage>();
    GError* err{};
    bool loaded = img->loadData(std::move(*contents), &err);

    if (err != nullptr) {
        string message = FS(_F("Could not load LaTeX PDF file: {1}") % err->message);
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

    if (std::abs(imgheight) > 1024 * std::numeric_limits<double>::epsilon()) {
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
    xoj_assert(this->isValidTex);
    xoj_assert(this->temporaryRender != nullptr);

    /* Das folgende wird erst am Ende gemacht... */
    Layer* layer = page->getSelectedLayer();
    XournalView* xournal = this->control->getWindow()->getXournal();
    auto pageNr = xournal->getCurrentPage();
    auto* view = xournal->getViewFor(pageNr);

    /* Clearing the old image and creating the new one creates two separate undo actions; I don't
       know yet how to merge that to one. (That bug was already present before.) */

    this->control->clearSelectionEndText();
    if (this->selectedElem) {
        auto sel = SelectionFactory::createFromElementOnActiveLayer(control, page, view, selectedElem);
        view->getXournal()->deleteSelection(sel.release());
        this->selectedElem = nullptr;
    }

    if (view->getPage() != page) {
        g_warning("Active page changed while you edited the tex code. Aborting.");
        return;
    }


    control->getUndoRedoHandler()->addUndoAction(
            std::make_unique<InsertUndoAction>(page, layer, this->temporaryRender.get()));

    // Select element
    auto selection =
            SelectionFactory::createFromFloatingElement(control, page, layer, view, std::move(this->temporaryRender));
    view->getXournal()->setSelection(selection.release());
}

void LatexController::cancelEditing() {
    // The original element is currently selected. This drops it back onto the page
    this->control->clearSelectionEndText();
}


void LatexController::insertLatex(PageRef page, Control* ctrl, double x, double y) {
    auto self = std::make_unique<LatexController>(ctrl);
    auto depStatus = self->findTexDependencies();
    if (!depStatus.success) {
        XojMsgBox::showErrorToUser(ctrl->getGtkWindow(), depStatus.errorMsg);
        return;
    }

    self->page = page;


    // Is there already a teximage?
    self->selectedElem = nullptr;

    // Should we reverse this loop to select the most recent teximage rather than the oldest?
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_TEXIMAGE || e->getType() == ELEMENT_TEXT) {
            GdkRectangle matchRect = {gint(x), gint(y), 1, 1};
            if (e->intersectsArea(&matchRect)) {
                self->selectedElem = e.get();
                //ti = dynamic_cast<const TexImage*>(e.get());
                break;
            }
        }
    }

    if (self->selectedElem) {
        xoj::util::Rectangle<double> rect = self->selectedElem->getSnappedBounds();
        self->posx = rect.x;
        self->posy = rect.y;

        if (auto* img = dynamic_cast<const TexImage*>(self->selectedElem)) {
            self->initialTex = img->getText();
            self->temporaryRender = img->cloneTexImage();
            self->isValidTex = true;
        } else if (auto* txt = dynamic_cast<const Text*>(self->selectedElem)) {
            self->initialTex = "\\text{" + txt->getText() + "}";
        } else {
            xoj_assert(false);
        }

        self->imgwidth = self->selectedElem->getElementWidth();
        self->imgheight = self->selectedElem->getElementHeight();

    } else {
        self->posx = x;
        self->posy = y;
    }



    showTexEditDialog(std::move(self));
}
