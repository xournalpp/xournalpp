#include "FileChooserFiltersHelper.h"

#include "util/i18n.h"

namespace xoj {
static void addMimeTypeFilter(GtkFileChooser* fc, const char* name, const char* mime) {
    GtkFileFilter* filterPdf = gtk_file_filter_new();
    gtk_file_filter_set_name(filterPdf, name);
    gtk_file_filter_add_mime_type(filterPdf, mime);
    gtk_file_chooser_add_filter(fc, filterPdf);
}

void addFilterAllFiles(GtkFileChooser* fc) {
    GtkFileFilter* filterAll = gtk_file_filter_new();
    gtk_file_filter_set_name(filterAll, _("All files"));
    gtk_file_filter_add_pattern(filterAll, "*");
    gtk_file_chooser_add_filter(fc, filterAll);
}

void addFilterSupported(GtkFileChooser* fc) {
    GtkFileFilter* filterSupported = gtk_file_filter_new();
    gtk_file_filter_set_name(filterSupported, _("Supported files"));
    gtk_file_filter_add_mime_type(filterSupported, "application/x-xojpp");
    gtk_file_filter_add_mime_type(filterSupported, "application/x-xopp");
    gtk_file_filter_add_mime_type(filterSupported, "application/x-xopt");
    gtk_file_filter_add_mime_type(filterSupported, "application/pdf");
    gtk_file_filter_add_pattern(filterSupported, "*.moj");  // MrWriter
    gtk_file_chooser_add_filter(fc, filterSupported);
}

void addFilterPdf(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("PDF files"), "application/pdf"); }
void addFilterXoj(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("Xournal files"), "application/x-xojpp"); }
void addFilterXopp(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("Xournal++ files"), "application/x-xopp"); }
void addFilterXopt(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("Xournal++ template"), "application/x-xopt"); }
void addFilterSvg(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("SVG graphics"), "image/svg+xml"); }
void addFilterPng(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("PNG graphics"), "image/png"); }
};  // namespace xoj
