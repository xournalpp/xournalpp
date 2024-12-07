#include "FileChooserFiltersHelper.h"

#include "control/jobs/BaseExportJob.h"
#include "util/MimeTypes.h"
#include "util/i18n.h"

namespace xoj {
static void addMimeTypeFilter(GtkFileChooser* fc, const char* name, const MimeType& mime) {
    GtkFileFilter* filterPdf = gtk_file_filter_new();
    gtk_file_filter_set_name(filterPdf, name);

    mime.addToFilter(filterPdf);

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

    xoj::Mime::XOJ.addToFilter(filterSupported);
    xoj::Mime::XOPP.addToFilter(filterSupported);
    xoj::Mime::XOPT.addToFilter(filterSupported);
    xoj::Mime::PDF.addToFilter(filterSupported);

    gtk_file_filter_add_pattern(filterSupported, "*.moj");  // MrWriter
    gtk_file_chooser_add_filter(fc, filterSupported);
}

void addFilterPdf(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("PDF files"), xoj::Mime::PDF); }
void addFilterXoj(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("Xournal files"), xoj::Mime::XOJ); }
void addFilterXopp(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("Xournal++ files"), xoj::Mime::XOPP); }
void addFilterXopt(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("Xournal++ template"), xoj::Mime::XOPT); }
void addFilterSvg(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("SVG graphics"), xoj::Mime::SVG); }
void addFilterPng(GtkFileChooser* fc) { addMimeTypeFilter(fc, _("PNG graphics"), xoj::Mime::PNG); }

void addFilterImages(GtkFileChooser* fc) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Image files"));
    gtk_file_filter_add_pixbuf_formats(filter);
    gtk_file_chooser_add_filter(fc, filter);
}
};  // namespace xoj
