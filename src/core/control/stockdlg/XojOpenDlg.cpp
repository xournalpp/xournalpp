#include "XojOpenDlg.h"

#include <string>  // for string

#include <gdk-pixbuf/gdk-pixbuf.h>  // for gdk_pixbuf_new_from_stream
#include <gio/gio.h>                // for g_input_stream_close, g_memor...
#include <glib-object.h>            // for g_object_unref, g_object_ref
#include <glib.h>                   // for gchar, g_error_free, g_get_ho...

#include "control/settings/Settings.h"  // for Settings
#include "util/PathUtil.h"              // for fromGFilename, toGFilename
#include "util/XojPreviewExtractor.h"   // for XojPreviewExtractor, PREVIEW_...
#include "util/i18n.h"                  // for _

XojOpenDlg::XojOpenDlg(GtkWindow* win, Settings* settings): win(win), settings(settings) {
    dialog = gtk_file_chooser_dialog_new(_("Open file"), win, GTK_FILE_CHOOSER_ACTION_OPEN, _("_Cancel"),
                                         GTK_RESPONSE_CANCEL, _("_Open"), GTK_RESPONSE_OK, nullptr);

    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

    fs::path currentFolder;
    if (!settings->getLastOpenPath().empty()) {
        currentFolder = settings->getLastOpenPath();
    } else {
        g_warning("lastOpenPath is not set!");
        currentFolder = g_get_home_dir();
    }
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Util::toGFilename(currentFolder).c_str());
}

XojOpenDlg::~XojOpenDlg() {
    if (dialog) {
        gtk_widget_destroy(dialog);
    }
    dialog = nullptr;
}

void XojOpenDlg::addFilterAllFiles() {
    GtkFileFilter* filterAll = gtk_file_filter_new();
    gtk_file_filter_set_name(filterAll, _("All files"));
    gtk_file_filter_add_pattern(filterAll, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);
}

void XojOpenDlg::addFilterPdf() {
    GtkFileFilter* filterPdf = gtk_file_filter_new();
    gtk_file_filter_set_name(filterPdf, _("PDF files"));
    gtk_file_filter_add_pattern(filterPdf, "*.pdf");
    gtk_file_filter_add_pattern(filterPdf, "*.PDF");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);
}

void XojOpenDlg::addFilterXoj() {
    GtkFileFilter* filterXoj = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXoj, _("Xournal files"));
    gtk_file_filter_add_pattern(filterXoj, "*.xoj");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);
}

void XojOpenDlg::addFilterXopp() {
    GtkFileFilter* filterXopp = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXopp, _("Xournal++ files"));
    gtk_file_filter_add_pattern(filterXopp, "*.xopp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXopp);
}

void XojOpenDlg::addFilterXopt() {
    GtkFileFilter* filterXopt = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXopt, _("Xournal++ template"));
    gtk_file_filter_add_pattern(filterXopt, "*.xopt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXopt);
}

auto XojOpenDlg::runDialog() -> fs::path {
    gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
        gtk_widget_destroy(dialog);
        dialog = nullptr;
        return fs::path{};
    }

    auto file = Util::fromGFilename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
    settings->setLastOpenPath(file.parent_path());
    return file;
}

auto XojOpenDlg::showOpenTemplateDialog() -> fs::path {
    addFilterAllFiles();
    addFilterXopt();

    return runDialog();
}

auto XojOpenDlg::showOpenDialog(bool pdf, bool& attachPdf) -> fs::path {
    if (!pdf) {
        GtkFileFilter* filterSupported = gtk_file_filter_new();
        gtk_file_filter_set_name(filterSupported, _("Supported files"));
        gtk_file_filter_add_pattern(filterSupported, "*.xoj");
        gtk_file_filter_add_pattern(filterSupported, "*.xopp");
        gtk_file_filter_add_pattern(filterSupported, "*.xopt");
        gtk_file_filter_add_pattern(filterSupported, "*.pdf");
        gtk_file_filter_add_pattern(filterSupported, "*.PDF");
        gtk_file_filter_add_pattern(filterSupported, "*.moj");  // MrWriter
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

        addFilterXoj();
        addFilterXopt();
        addFilterXopp();
    }

    addFilterPdf();
    addFilterAllFiles();

    GtkWidget* attachOpt = nullptr;
    if (pdf) {
        attachOpt = gtk_check_button_new_with_label(_("Attach file to the journal"));
        g_object_ref(attachOpt);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attachOpt), false);
        gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), attachOpt);
    }

    GtkWidget* image = gtk_image_new();
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
    g_signal_connect(dialog, "update-preview", G_CALLBACK(updatePreviewCallback), nullptr);

    auto lastOpenPath = this->settings->getLastOpenPath();
    if (!lastOpenPath.empty()) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(this->dialog), Util::toGFilename(lastOpenPath).c_str());
    }

    auto lastSavePath = this->settings->getLastSavePath();
    if (!lastSavePath.empty()) {
        gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(this->dialog), lastSavePath.u8string().c_str(), nullptr);
    }

    fs::path file = runDialog();

    if (attachOpt) {
        attachPdf = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attachOpt));
        g_object_unref(attachOpt);
    }

    if (!file.empty()) {
        g_message("lastOpenPath set");
        this->settings->setLastOpenPath(file.parent_path());
    }

    return file;
}

void XojOpenDlg::updatePreviewCallback(GtkFileChooser* fileChooser, void* userData) {
    auto filepath = Util::fromGFilename(gtk_file_chooser_get_preview_filename(fileChooser));

    if (filepath.empty()) {
        gtk_file_chooser_set_preview_widget_active(fileChooser, false);
        return;
    }

    if (!Util::hasXournalFileExt(filepath)) {
        gtk_file_chooser_set_preview_widget_active(fileChooser, false);
        return;
    }

    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(filepath);

    if (result != PREVIEW_RESULT_IMAGE_READ) {
        gtk_file_chooser_set_preview_widget_active(fileChooser, false);
        return;
    }

    GError* error = nullptr;
    gsize dataLen = 0;
    unsigned char* imageData = extractor.getData(dataLen);

    GInputStream* in = g_memory_input_stream_new_from_data(imageData, dataLen, nullptr);
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(in, nullptr, &error);
    if (error != nullptr) {
        g_warning("Could not load preview image, error: %s\n", error->message);
        g_error_free(error);
    }

    g_input_stream_close(in, nullptr, nullptr);

    if (pixbuf) {
        GtkWidget* image = gtk_file_chooser_get_preview_widget(fileChooser);
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
        g_object_unref(pixbuf);
        gtk_file_chooser_set_preview_widget_active(fileChooser, true);
    }
}
