#include "ExtEdLatexDialog.h"

#include <fstream>  // for ofstream
#include <system_error>

#include <gio/gio.h>      // for g_subprocess*
#include <glib-object.h>  // for g_object_unref, g_clear_object
#include <glib.h>
#include <gtk/gtk.h>  // for gtk_widget_set_sensitive

#include "control/Control.h"
#include "control/LatexController.h"
#include "control/settings/LatexSettings.h"
#include "gui/Builder.h"
#include "gui/GladeSearchpath.h"
#include "util/PathUtil.h"         // for readString
#include "util/XojMsgBox.h"        // for XojMsgBox
#include "util/i18n.h"             // for FS, _, _F, N_
#include "util/raii/GLibGuards.h"  // for GErrorGuard, GStrvGuard

#include "AbstractLatexDialog.h"

constexpr auto UI_FILE_NAME = "extEdTexDialog.glade";
constexpr auto UI_DIALOG_ID = "extEdTexDialog";

ExtEdLatexDialog::ExtEdLatexDialog(GladeSearchpath* gladeSearchPath, std::unique_ptr<LatexController> ctrl):
        AbstractLatexDialog(std::move(ctrl)) {
    Builder builder(gladeSearchPath, UI_FILE_NAME);
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_ID)));
    populateStandardWidgetsFromBuilder(builder);
    btContEdit = GTK_BUTTON(builder.get("btContEdit"));
    connectStandardSignals();

    tempf = texCtrl->texTmpDir / "edit.";

    // Append file extension.
    tempf += texCtrl->settings.temporaryFileExt;

    // Write initial content
    std::ofstream os{tempf};
    if (!texCtrl->initialTex.empty()) {
        os << texCtrl->initialTex;
    } else {
        os << texCtrl->settings.defaultText;
    }
    os.close();

    g_signal_connect(getWindow(), "show", G_CALLBACK(+[](GtkWidget*, gpointer d) {
                         auto self = static_cast<ExtEdLatexDialog*>(d);
                         if (!self->texCtrl->temporaryRender) {
                             // Trigger an asynchronous compilation if we are not using a preexisting TexImage
                             // Keep this after popup.show() so that if an error message is to be displayed (e.g.
                             // missing Tex executable), it'll appear on top of the LatexDialog.
                             LatexController::handleTexChanged(self->texCtrl.get());
                         }

                         self->openEditor();
                     }),
                     this);
    g_signal_connect(btContEdit, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
                         auto* self = static_cast<ExtEdLatexDialog*>(d);
                         self->openEditor();
                     }),
                     this);
}

ExtEdLatexDialog::~ExtEdLatexDialog() {
    try {
        fs::remove(tempf);
    } catch (const std::system_error& e) {
        g_warning("Failed to remove LaTeX edit file: %s", e.what());
    }
    if (editorCancellable) {
        g_cancellable_cancel(editorCancellable);
        g_object_unref(editorCancellable);
    }
};

void ExtEdLatexDialog::setCompilationStatus(bool isTexValid, bool isCompilationDone,
                                            const std::string& compilationOutput) {
    if (texCtrl->settings.externalEditorAutoConfirm && !editorCancellable && isTexValid && isCompilationDone) {
        texCtrl->insertTexImage();
        gtk_window_close(getWindow());
        return;
    }

    AbstractLatexDialog::setCompilationStatus(isTexValid, isCompilationDone, compilationOutput);
};

void ExtEdLatexDialog::openEditor() {
    if (editorCancellable) {
        return;
    }

    xoj::util::GErrorGuard err{};
    xoj::util::GStrvGuard argv{};

    std::string editorCmd = texCtrl->settings.externalEditorCmd;
    if (editorCmd.empty()) {
        auto maybeEditor = g_getenv("EDITOR");
        if (!maybeEditor) {
            XojMsgBox::showErrorToUser(
                    getWindow(),
                    "No external editor configured and no $EDITOR environment variable set. Please configure an "
                    "external editor command or disable the external editor functionality to use a built-in one.");
            return;
        }
        editorCmd = maybeEditor;
    }
    editorCmd += " '";
    editorCmd += tempf.u8string();
    editorCmd += "'";

    if (!g_shell_parse_argv(editorCmd.c_str(), nullptr, xoj::util::out_ptr(argv), xoj::util::out_ptr(err))) {
        XojMsgBox::showErrorToUser(getWindow(), FS(_F("Failed to parse external editor command: {1}") % err->message));
        return;
    }

    xoj::util::GObjectSPtr<GSubprocessLauncher> launcher(g_subprocess_launcher_new(G_SUBPROCESS_FLAGS_NONE),
                                                         xoj::util::adopt);

    Color textColor = texCtrl->control->getToolHandler()->getTool(TOOL_TEXT).getColor();
    std::string colorStr = Util::rgb_to_hex_string(textColor).substr(1);

    g_subprocess_launcher_setenv(launcher.get(), "XPP_TEXT_COLOR", colorStr.c_str(), TRUE);

    xoj::util::GObjectSPtr<GSubprocess> process(
            g_subprocess_launcher_spawnv(launcher.get(), argv.get(), xoj::util::out_ptr(err)), xoj::util::adopt);

    if (err) {
        XojMsgBox::showErrorToUser(getWindow(), FS(_F("Could not spawn editor: {1}") % err->message));
        return;
    }

    gtk_widget_set_sensitive(GTK_WIDGET(btContEdit), false);

    editorCancellable = g_cancellable_new();
    g_subprocess_wait_async(process.get(), editorCancellable, reinterpret_cast<GAsyncReadyCallback>(editorWaitCallback),
                            this);
}

void ExtEdLatexDialog::editorWaitCallback(GObject* processObj, GAsyncResult* res, ExtEdLatexDialog* self) {
    auto process = G_SUBPROCESS(processObj);

    xoj::util::GErrorGuard err;
    if (!g_subprocess_wait_finish(process, res, xoj::util::out_ptr(err))) {
        // If we cancelled the operation, that's because the user closed the popup while the editor was still open. We
        // must not interact with self now, because that's likely already been destroyed. We kill the editor and then
        // return.
        if (g_error_matches(err.get(), G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
#ifdef G_OS_UNIX
            g_subprocess_send_signal(process, SIGTERM);
#else
            // The g_subprocess_send_signal API is unavailable on non-unix platforms, so we use the cross-platform
            // g_subprocess_force_exit function. We don't use it on supported platforms so we can send a "nicer" exit
            // signal if available, as g_subprocess_force_exit sends SIGKILL on Unices.
            g_subprocess_force_exit(process);
#endif
            return;
        }

        XojMsgBox::showErrorToUser(self->getWindow(), FS(_F("Could not wait for editor: {1}") % err->message));
        return;
    }

    g_clear_object(&self->editorCancellable);
    gtk_widget_set_sensitive(GTK_WIDGET(self->btContEdit), true);

    auto status = g_subprocess_get_status(process);
    if (status != 0) {
        XojMsgBox::showErrorToUser(self->getWindow(), FS(_F("Editor exited with bad status {1}") % status));
        return;
    }

    auto content = self->getBufferContents();

    self->texCtrl->triggerImageUpdate(content);
}

std::string ExtEdLatexDialog::getBufferContents() {
    auto content = Util::readString(tempf);
    if (!content) {
        return std::string();
    }

    // Remove a possible trailing newline
    if (!content->empty() && content->back() == '\n') {
        content->pop_back();
    }

    return *content;
}
