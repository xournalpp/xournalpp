#include "util/XojMsgBox.h"

#include <utility>  // for move

#include <glib-object.h>  // for g_object_set_property, g_value_init, g_valu...
#include <glib.h>         // for g_free, g_markup_escape_text, g_error_free

#include "util/PopupWindowWrapper.h"
#include "util/Util.h"
#include "util/gtk4_helper.h"
#include "util/i18n.h"  // for _, FS, _F
#include "util/raii/CStringWrapper.h"

#ifdef _WIN32
// Needed for help dialog workaround on Windows; see XojMsgBox::showHelp
#include <windows.h>
// <windows.h> must be included first
#include <shellapi.h>  // for ShellExecute
#endif

GtkWindow* defaultWindow = nullptr;

XojMsgBox::XojMsgBox(GtkDialog* dialog, xoj::util::move_only_function<void(int)> callback, CallbackPolicy pol):
        window(reinterpret_cast<GtkWindow*>(dialog)), callback(std::move(callback)), policy(pol) {
    this->signalId =
            g_signal_connect(dialog, "response", G_CALLBACK(+[](GtkDialog* dialog, int response, gpointer data) {
                                 auto* self = static_cast<XojMsgBox*>(data);

                                 if (self->policy == IMMEDIATE) {
                                     self->callback(response);
                                 } else {  // POSTPONED
                                     // We sometimes need to call gtk_window_close() before invoking the callback,
                                     // because if the callback pops up another dialog, the first one won't close...
                                     // But since gtk_window_close() triggers the destruction of *self, we first move
                                     // the callback
                                     Util::execWhenIdle([cb = std::move(self->callback), r = response]() { cb(r); });
                                 }

                                 // Closing the window causes another "response" signal, which we want to ignore
                                 g_signal_handler_disconnect(dialog, self->signalId);
                                 gtk_window_close(self->getWindow());  // Destroys *self. Beware!
                             }),
                             this);
}


/**
 * Set window for messages without window
 */
void XojMsgBox::setDefaultWindow(GtkWindow* win) { defaultWindow = win; }

void XojMsgBox::showMarkupMessageToUser(GtkWindow* win, const std::string_view& markupTitle, const std::string& msg,
                                        GtkMessageType type) {
    if (win == nullptr) {
        win = defaultWindow;
    }

    GtkWidget* dialog = gtk_message_dialog_new_with_markup(win, GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK, nullptr);

    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), markupTitle.data());

    if (!msg.empty()) {
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", msg.c_str());
    }

    xoj::popup::PopupWindowWrapper<XojMsgBox> popup(GTK_DIALOG(dialog));
    popup.show(win);
}

void XojMsgBox::showMessageToUser(GtkWindow* win, const std::string& title, const std::string& msg,
                                  GtkMessageType type) {
    auto escapedTitle = xoj::util::OwnedCString::assumeOwnership(g_markup_escape_text(title.c_str(), -1));
    showMarkupMessageToUser(win, escapedTitle.get(), msg, type);
}

void XojMsgBox::showMessageToUser(GtkWindow* win, const std::string& msg, GtkMessageType type) {
    showMessageToUser(win, msg, std::string(), type);
}

void XojMsgBox::showErrorToUser(GtkWindow* win, const std::string& msg) {
    showMessageToUser(win, msg, GTK_MESSAGE_ERROR);
    g_warning("%s", msg.c_str());
}

void XojMsgBox::askQuestion(GtkWindow* win, const std::string& maintext, const std::string& secondarytext,
                            const std::vector<Button>& buttons, xoj::util::move_only_function<void(int)> callback) {

    auto formattedMsg = xoj::util::OwnedCString::assumeOwnership(g_markup_escape_text(maintext.c_str(), -1));
    askQuestionWithMarkup(win, std::string_view(formattedMsg), secondarytext, buttons, std::move(callback));
}

void XojMsgBox::askQuestionWithMarkup(GtkWindow* win, std::string_view maintext, const std::string& secondarytext,
                                      const std::vector<Button>& buttons,
                                      xoj::util::move_only_function<void(int)> callback) {
    if (win == nullptr) {
        win = defaultWindow;
    }

    GtkWidget* dialog =
            gtk_message_dialog_new_with_markup(win, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, nullptr);

    for (auto& b: buttons) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), b.label.c_str(), b.response);
    }

    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), maintext.data());

    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", secondarytext.c_str());

    xoj::popup::PopupWindowWrapper<XojMsgBox> popup(GTK_DIALOG(dialog), std::move(callback));
    popup.show(win);
}

void XojMsgBox::showErrorAndQuit(std::string& msg, int exitCode) {
    GtkWidget* dialog = gtk_message_dialog_new_with_markup(defaultWindow, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                           GTK_BUTTONS_OK, nullptr);

    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), msg.c_str());

    // We use a hack to ensure the app does not exit until the user has read the error message and closed the popup
    bool done = false;

    xoj::popup::PopupWindowWrapper<XojMsgBox> popup(GTK_DIALOG(dialog), [&done](int) { done = true; });

    popup.show(defaultWindow);

    while (!done) {
        // Let the main loop run so the popup pops up and can be interacted with
        g_main_context_iteration(g_main_context_default(), true);
    }

    exit(exitCode);
}

void XojMsgBox::showPluginMessage(const std::string& pluginName, const std::string& msg, bool error) {
    auto header = std::string("Xournal++ Plugin «") + pluginName + "»";
    auto escapedHeader = xoj::util::OwnedCString::assumeOwnership(g_markup_escape_text(header.c_str(), -1));
    header = (error ? std::string("<b>Error in </b>") : "") + escapedHeader.get();

    showMarkupMessageToUser(nullptr, header, msg, error ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO);
}

auto XojMsgBox::askPluginQuestion(const std::string& pluginName, const std::string& msg,
                                  const std::vector<Button>& buttons, bool error) -> int {
    /*
     * Todo(gtk4): Remove this function entirely.
     */
    std::string header = "<i>Warning: The plugin interface function msgbox() is deprecated and will soon be removed. "
                         "Please adapt your plugin to use the function openDialog() instead</i>\n\n";
    header += (error ? std::string("<b>Error in </b>") : "") + std::string("Xournal++ Plugin «") + pluginName + "»";

    GtkWidget* dialog = gtk_message_dialog_new_with_markup(defaultWindow, GTK_DIALOG_MODAL,
                                                           error ? GTK_MESSAGE_ERROR : GTK_MESSAGE_QUESTION,
                                                           GTK_BUTTONS_NONE, nullptr);

    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), header.c_str());

    if (defaultWindow != nullptr) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), defaultWindow);
    }

    GValue val = G_VALUE_INIT;
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_string(&val, msg.c_str());
    g_object_set_property(G_OBJECT(dialog), "secondary-text", &val);
    g_value_unset(&val);

    for (auto& b: buttons) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), b.label.c_str(), b.response);
    }

    // We use a hack to ensure the app does not exit until the user has read the error message and closed the popup
    bool done = false;
    int res = 0;

    xoj::popup::PopupWindowWrapper<XojMsgBox> popup(GTK_DIALOG(dialog), [&done, &res](int r) {
        done = true;
        res = r;
    });

    popup.show(defaultWindow);

    while (!done) {
        // Let the main loop run so the popup pops up and can be interacted with
        g_main_context_iteration(g_main_context_default(), true);
    }
    return res;
}

void XojMsgBox::replaceFileQuestion(GtkWindow* win, fs::path file,
                                    xoj::util::move_only_function<void(const fs::path&)> writeTofile) {
    if (!fs::exists(file)) {
        writeTofile(file);
        return;
    }

    GtkWidget* dialog = gtk_message_dialog_new(
            win, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s",
            FS(FORMAT_STR("The file {1} already exists! Do you want to replace it?") % file.filename().u8string())
                    .c_str());
    if (win != nullptr) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
    }
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Select another name"), GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Replace"), GTK_RESPONSE_OK);

    xoj::popup::PopupWindowWrapper<XojMsgBox> popup(
            GTK_DIALOG(dialog), [overwrite = std::move(writeTofile), file = std::move(file)](int response) mutable {
                if (response == GTK_RESPONSE_OK) {
                    overwrite(file);
                }
            });
    popup.show(win);
}

constexpr auto* XOJ_HELP = "https://xournalpp.github.io/community/help/";

void XojMsgBox::showHelp(GtkWindow* win) {
#ifdef _WIN32  // TODO Do we still need a separate handling for Windows?
    // gvfs is not in MSYS repositories, so we can't use gtk_show_uri.
    // Instead, we use the native API instead.
    ShellExecute(nullptr, "open", XOJ_HELP, nullptr, nullptr, SW_SHOW);
#else
    gtk_show_uri(win, XOJ_HELP, GDK_CURRENT_TIME);
#endif
}
