#include "util/XojMsgBox.h"

#include <utility>  // for pair

#include <glib-object.h>  // for g_object_set_property, g_value_init, g_valu...
#include <glib.h>         // for g_free, g_markup_escape_text, g_error_free

#include "../core/gui/PopupWindowWrapper.h"
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

XojMsgBox::XojMsgBox(GtkDialog* dialog, std::function<void(int)> callback):
        window(reinterpret_cast<GtkWindow*>(dialog)), callback(std::move(callback)) {
    g_signal_connect(dialog, "response", G_CALLBACK(+[](GtkDialog* dialog, int response, XojMsgBox* self) {
                         self->callback(response);
                         gtk_window_close(reinterpret_cast<GtkWindow*>(dialog));
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
                            const std::vector<Button>& buttons, std::function<void(int)> callback) {

    auto formattedMsg = xoj::util::OwnedCString::assumeOwnership(g_markup_escape_text(maintext.c_str(), -1));
    askQuestionWithMarkup(win, std::string_view(formattedMsg), secondarytext, buttons, std::move(callback));
}

void XojMsgBox::askQuestionWithMarkup(GtkWindow* win, std::string_view maintext, const std::string& secondarytext,
                                      const std::vector<Button>& buttons, std::function<void(int)> callback) {
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

    std::function<void(int)>* data = new std::function<void(int)>(std::move(callback));
    g_signal_connect_data(dialog, "response",
                          G_CALLBACK(+[](GtkDialog* dialog, int response, std::function<void(int)>* callback) {
                              (*callback)(response);
                              gtk_window_close(GTK_WINDOW(dialog));
                          }),
                          data, GClosureNotify(+[](std::function<void(int)>* self) { delete self; }),
                          GConnectFlags(0U));  // G_CONNECT_DEFAULT = GConnectFlags(0U) only defined in glib 2.74

    xoj::popup::PopupWindowWrapper<XojMsgBox> popup(GTK_DIALOG(dialog));
    popup.show(win);
}

void XojMsgBox::showErrorAndQuit(std::string& msg, int exitCode) {
    /*
     * This should be used for fatal errors, typically in early GUI startup (missing UI main file or so).
     *
     * Todo(gtk4): Figure out how to use a non-blocking dialog that would survive past the `exit` call.
     * Putting the `exit()` in a callback is no good, as normal execution would continue in the background, most likely
     * leading to a SegFault: that would again kill the dialog.
     */
    GtkWidget* dialog = gtk_message_dialog_new_with_markup(defaultWindow, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                           GTK_BUTTONS_OK, nullptr);

    auto formattedMsg = xoj::util::OwnedCString::assumeOwnership(g_markup_escape_text(msg.c_str(), -1));
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), formattedMsg.get());
    if (defaultWindow != nullptr) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), defaultWindow);
    }
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

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

    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return res;
}

auto XojMsgBox::replaceFileQuestion(GtkWindow* win, const std::string& msg) -> int {
    GtkWidget* dialog =
            gtk_message_dialog_new(win, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s", msg.c_str());
    if (win != nullptr) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
    }
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Select another name"), GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Replace"), GTK_RESPONSE_OK);
    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return res;
}

constexpr auto* XOJ_HELP = "https://xournalpp.github.io/community/help/";

void XojMsgBox::showHelp(GtkWindow* win) {
#ifdef _WIN32
    // gvfs is not in MSYS repositories, so we can't use gtk_show_uri.
    // Instead, we use the native API instead.
    ShellExecute(nullptr, "open", XOJ_HELP, nullptr, nullptr, SW_SHOW);
#else
    GError* error = nullptr;
    gtk_show_uri(gtk_window_get_screen(win), XOJ_HELP, gtk_get_current_event_time(), &error);

    if (error) {
        std::string msg = FS(_F("There was an error displaying help: {1}") % error->message);
        XojMsgBox::showErrorToUser(win, msg);

        g_error_free(error);
    }
#endif
}
