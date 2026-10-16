#include "NativeFileChooserHelper.h"

#include "util/PathUtil.h"
#include "util/i18n.h"
#include "util/raii/GObjectSPtr.h"

namespace xoj::NativeFileChooser {

bool isAvailable() {
#if GTK_CHECK_VERSION(3, 20, 0) && (defined(_WIN32) || defined(__APPLE__))
    return true;
#else
    return false;
#endif
}

#if GTK_CHECK_VERSION(3, 20, 0) && (defined(_WIN32) || defined(__APPLE__))
class NativeDialog {
public:
    NativeDialog(GtkWindow* parent, const char* title, GtkFileChooserAction action, const char* acceptLabel,
                 ConfigureCallback configure, FileSelectedCallback callback):
            dialog(gtk_file_chooser_native_new(title, parent, action, acceptLabel, _("_Cancel")), xoj::util::adopt),
            callback(std::move(callback)) {
        gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(dialog.get()), TRUE);
        configure(GTK_FILE_CHOOSER(dialog.get()));
    }

    void show() {
        signalId = g_signal_connect(dialog.get(), "response",
                                    G_CALLBACK(+[](GtkNativeDialog* dialog, int response, gpointer data) {
                                        auto* self = static_cast<NativeDialog*>(data);
                                        self->handleResponse(dialog, response);
                                    }),
                                    this);
        gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog.get()));
    }

private:
    void handleResponse(GtkNativeDialog* nativeDialog, int response) {
        std::optional<fs::path> path;
        std::optional<std::string> filterName;

        if (response == GTK_RESPONSE_ACCEPT) {
            auto* chooser = GTK_FILE_CHOOSER(nativeDialog);
            path = Util::fromGFile(
                    xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(chooser), xoj::util::adopt).get());

            if (auto* filter = gtk_file_chooser_get_filter(chooser); filter) {
                if (const char* name = gtk_file_filter_get_name(filter); name) {
                    filterName = name;
                }
            }
        }

        auto cb = std::move(callback);
        g_signal_handler_disconnect(dialog.get(), signalId);
        gtk_native_dialog_destroy(GTK_NATIVE_DIALOG(dialog.get()));
        dialog.reset();

        cb(std::move(path), std::move(filterName));
        delete this;
    }

    xoj::util::GObjectSPtr<GtkFileChooserNative> dialog;
    FileSelectedCallback callback;
    gulong signalId = 0;
};
#endif

void show(GtkWindow* parent, const char* title, GtkFileChooserAction action, const char* acceptLabel,
          ConfigureCallback configure, FileSelectedCallback callback) {
#if GTK_CHECK_VERSION(3, 20, 0) && (defined(_WIN32) || defined(__APPLE__))
    auto* dialog = new NativeDialog(parent, title, action, acceptLabel, std::move(configure), std::move(callback));
    dialog->show();
#else
    static_cast<void>(parent);
    static_cast<void>(title);
    static_cast<void>(action);
    static_cast<void>(acceptLabel);
    static_cast<void>(configure);
    callback(std::nullopt, std::nullopt);
#endif
}

};  // namespace xoj::NativeFileChooser
