#include "FileChooserFiltersHelper.h"

#include "control/jobs/BaseExportJob.h"
#include "util/i18n.h"

namespace xoj {
static void addMimeTypeFilter(GtkFileChooser* fc, const char* name, const ExportType& mime) {
    GtkFileFilter* filterPdf = gtk_file_filter_new();
    gtk_file_filter_set_name(filterPdf, name);
#ifdef G_OS_WIN32
    gtk_file_filter_add_pattern(filterPdf, ('*' + mime.extension).c_str());
#else
    gtk_file_filter_add_mime_type(filterPdf, mime.mimeType.c_str());
#endif
    gtk_file_chooser_add_filter(fc, filterPdf);
}

void addFilterAllFiles(GtkFileChooser* fc) {
    GtkFileFilter* filterAll = gtk_file_filter_new();
    gtk_file_filter_set_name(filterAll, _("All files"));
    gtk_file_filter_add_pattern(filterAll, "*");
    gtk_file_chooser_add_filter(fc, filterAll);
}

void addFilterSupported(GtkFileChooser* fc) {
    const char* desktop_env = g_getenv("XDG_CURRENT_DESKTOP");
    const char* portal = g_getenv("GTK_USE_PORTAL");

    // KDE dolphin automatically adds Supported files
    if (!desktop_env || g_strcmp0(desktop_env, "KDE") != 0 || !portal || g_strcmp0(portal, "1") != 0) {
        GtkFileFilter* filterSupported = gtk_file_filter_new();
        gtk_file_filter_set_name(filterSupported, _("Supported files"));
#ifdef G_OS_WIN32
        gtk_file_filter_add_pattern(filterSupported, "*.xoj");
        gtk_file_filter_add_pattern(filterSupported, "*.xopp");
        gtk_file_filter_add_pattern(filterSupported, "*.xopt");
        gtk_file_filter_add_pattern(filterSupported, "*.pdf");
#else
        gtk_file_filter_add_mime_type(filterSupported, "application/x-xojpp");
        gtk_file_filter_add_mime_type(filterSupported, "application/x-xopp");
        gtk_file_filter_add_mime_type(filterSupported, "application/x-xopt");
        gtk_file_filter_add_mime_type(filterSupported, "application/pdf");
#endif
        gtk_file_filter_add_pattern(filterSupported, "*.moj");  // MrWriter
        gtk_file_chooser_add_filter(fc, filterSupported);
    }
}

void addFilterPdf(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("PDF files"), ExportType(".pdf", "application/pdf")); }
void addFilterXoj(GtkFileChooser* fc) {
    addMimeTypeFilter(fc, _("Xournal files"), ExportType(".xoj", "application/x-xojpp"));
}
void addFilterXopp(GtkFileChooser* fc) {
    addMimeTypeFilter(fc, _("Xournal++ files"), ExportType(".xopp", "application/x-xopp"));
}
void addFilterXopt(GtkFileChooser* fc) {
    addMimeTypeFilter(fc, _("Xournal++ template"), ExportType(".xopt", "application/x-xopt"));
}
void addFilterSvg(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("SVG graphics"), ExportType(".svg", "image/svg+xml")); }
void addFilterPng(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("PNG graphics"), ExportType(".png", "image/png")); }

void addFilterImages(GtkFileChooser* fc) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Image files"));
    gtk_file_filter_add_pixbuf_formats(filter);
    gtk_file_chooser_add_filter(fc, filter);
}
};  // namespace xoj
