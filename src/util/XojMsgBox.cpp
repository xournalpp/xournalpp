#include "util/XojMsgBox.h"

#include <functional>
#include <future>
#include <type_traits>
#include <utility>

#include "util/i18n.h"

#include "GtkDialogUtil.h"
#include "Util.h"

#ifdef _WIN32
// Needed for help dialog workaround on Windows; see XojMsgBox::showHelp
#include <shlwapi.h>
#endif

constexpr auto DEFAULT_FLAGS = GtkDialogFlags(GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL);


struct WindowHolder {
    GtkWindow* defaultWindow{};
};

static auto getWindowHolder() -> WindowHolder& {
    static WindowHolder wh{};
    return wh;
}

static void response_connected_fn(GtkDialog* d, gint response, std::function<void(GtkDialog*, gint)>* fn) {
    (*fn)(d, response);
    gtk_window_destroy(GTK_WINDOW(d));  // delete window
    delete fn;                          // delete self
}

template <class Fn>
static void connect_fn_to_dialog(GtkWidget* dialog, Fn&& on_response) {
    auto* fn = new std::remove_reference_t<Fn>(std::forward<Fn>(on_response));
    g_signal_connect(dialog, "response", GCallback(response_connected_fn), fn);
}

/**
 * Set window for messages without window
 */
void XojMsgBox::setDefaultWindow(GtkWindow* win) { getWindowHolder().defaultWindow = win; }


void XojMsgBox::showErrorToUser(GtkWindow* win, std::string msg) {
    if (win == nullptr) {
        win = getWindowHolder().defaultWindow;
    }
    g_assert(win);
    // Append this after the current execution, only way to make the dialog modal
    Util::execInUiThread([=, msg = std::move(msg)] {
        auto flags = GtkDialogFlags(GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL);
        GtkWidget* dialog = gtk_message_dialog_new_with_markup(win, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, nullptr);

        char* formattedMsg = g_markup_escape_text(msg.c_str(), msg.size());
        gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), formattedMsg);
        g_free(formattedMsg);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
        //
        g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), dialog);
        gtk_widget_set_visible(GTK_WIDGET(dialog), true);
    });
}

void XojMsgBox::showPluginMessage(GtkWindow* parent, const std::string& pluginName, const std::string& msg,
                                  const std::map<int, std::string>& button, bool error,
                                  std::function<void(GtkDialog*, gint)>&& on_response) {
    auto header = std::string("Xournal++ Plugin »") + pluginName + "«";
    if (error) {
        header = "<b>Error in </b>" + header;
    }
    if (parent == nullptr) {
        parent = getWindowHolder().defaultWindow;
        g_assert(parent);
    }

    GtkWidget* dialog = gtk_message_dialog_new_with_markup(getWindowHolder().defaultWindow, DEFAULT_FLAGS,
                                                           GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, nullptr);
    char* formattedHeader = g_markup_escape_text(header.c_str(), -1);
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), formattedHeader);
    g_free(formattedHeader);


    GValue val = G_VALUE_INIT;
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_string(&val, msg.c_str());
    g_object_set_property(G_OBJECT(dialog), "secondary-text", &val);
    g_value_unset(&val);

    for (auto& kv: button) { gtk_dialog_add_button(GTK_DIALOG(dialog), kv.second.c_str(), kv.first); }

    connect_fn_to_dialog(dialog, std::move(on_response));
    gtk_window_set_transient_for(GTK_WINDOW(dialog), getWindowHolder().defaultWindow);
    gtk_widget_show(GTK_WIDGET(dialog));
}

void XojMsgBox::replaceFileQuestion(GtkWindow* win, std::filesystem::path const& path,
                                    std::function<void(GtkDialog*, gint)>&& on_response) {
    std::string msg =
            FS(FORMAT_STR("The file {1} already exists! Do you want to replace it?") % path.filename().u8string());
    if (win == nullptr) {
        win = getWindowHolder().defaultWindow;
        g_assert(win);
    }

    GtkWidget* dialog =
            gtk_message_dialog_new(win, DEFAULT_FLAGS, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s", msg.c_str());
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Select another name"), GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Replace"), GTK_RESPONSE_OK);
    //
    gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
    connect_fn_to_dialog(dialog, std::move(on_response));
    gtk_widget_show(GTK_WIDGET(dialog));
    return;
}

constexpr auto* XOJ_HELP = "https://xournalpp.github.io/community/help/";

void XojMsgBox::showHelp(GtkWindow* win) {
#ifdef _WIN32
    // gvfs is not in MSYS repositories, so we can't use gtk_show_uri.
    // Instead, we use the native API instead.
    ShellExecute(nullptr, "open", XOJ_HELP, nullptr, nullptr, SW_SHOW);
#else
    gtk_show_uri(win, XOJ_HELP, GDK_CURRENT_TIME);
#endif
}
