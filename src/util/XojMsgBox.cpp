#include "XojMsgBox.h"

#include "i18n.h"

#ifdef _WIN32
// Needed for help dialog workaround on Windows; see XojMsgBox::showHelp
#include <shlwapi.h>
#endif

GtkWindow* defaultWindow = nullptr;

/**
 * Set window for messages without window
 */
void XojMsgBox::setDefaultWindow(GtkWindow* win) { defaultWindow = win; }


void XojMsgBox::showErrorToUser(GtkWindow* win, const string& msg) {
    if (win == nullptr) {
        win = defaultWindow;
    }

    GtkWidget* dialog =
            gtk_message_dialog_new_with_markup(win, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, nullptr);
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), msg.c_str());
    if (win != nullptr) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
    }
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

auto XojMsgBox::showPluginMessage(const string& pluginName, const string& msg, const map<int, string>& button,
                                  bool error) -> int {
    string header = string("Xournal++ Plugin «") + pluginName + "»";

    if (error) {
        header = "<b>Error in </b>" + header;
    }

    GtkWidget* dialog = gtk_message_dialog_new_with_markup(defaultWindow, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
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

    for (auto& kv: button) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), kv.second.c_str(), kv.first);
    }

    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return res;
}

auto XojMsgBox::replaceFileQuestion(GtkWindow* win, const string& msg) -> int {
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

constexpr auto* XOJ_HELP = "https://github.com/xournalpp/xournalpp/wiki/User-Manual";

void XojMsgBox::showHelp(GtkWindow* win) {
#ifdef _WIN32
    // gvfs is not in MSYS repositories, so we can't use gtk_show_uri.
    // Instead, we use the native API instead.
    ShellExecute(nullptr, "open", XOJ_HELP, nullptr, nullptr, SW_SHOW);
#else
    GError* error = nullptr;
    gtk_show_uri(gtk_window_get_screen(win), XOJ_HELP, gtk_get_current_event_time(), &error);

    if (error) {
        string msg = FS(_F("There was an error displaying help: {1}") % error->message);
        XojMsgBox::showErrorToUser(win, msg);

        g_error_free(error);
    }
#endif
}
