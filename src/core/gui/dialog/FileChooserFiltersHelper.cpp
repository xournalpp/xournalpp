#include "FileChooserFiltersHelper.h"

#include "util/i18n.h"

namespace xoj {
void addFilterAllFiles(GtkFileChooser* fc) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("All files"));
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(fc, filter);
}

void addFilterSupported(GtkFileChooser* fc) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Supported files"));
#ifndef _WIN32
    gtk_file_filter_add_mime_type(filter, FileTypes::XOJ.mimetype);
    gtk_file_filter_add_mime_type(filter, FileTypes::XOPP.mimetype);
    gtk_file_filter_add_mime_type(filter, FileTypes::XOPT.mimetype);
    gtk_file_filter_add_mime_type(filter, FileTypes::PDF.mimetype);
#else
    gtk_file_filter_add_pattern(filter, (std::string("*.") + FileTypes::XOJ.extension).c_str());
    gtk_file_filter_add_pattern(filter, (std::string("*.") + FileTypes::XOPP.extension).c_str());
    gtk_file_filter_add_pattern(filter, (std::string("*.") + FileTypes::XOPT.extension).c_str());
    gtk_file_filter_add_pattern(filter, (std::string("*.") + FileTypes::PDF.extension).c_str());
#endif
    gtk_file_filter_add_pattern(filter, "*.moj");  // MrWriter
    gtk_file_chooser_add_filter(fc, filter);
}

void addFilterForFile(GtkFileChooser* fc, const FileType& f) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, fetch_translation(f.description));
#ifndef _WIN32
    gtk_file_filter_add_mime_type(filter, f.mimetype);
#else
    gtk_file_filter_add_pattern(filter, (std::string("*.") + f.extension).c_str());
#endif
    gtk_file_chooser_add_filter(fc, filter);
}

void addFilterImages(GtkFileChooser* fc) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Image files"));
    gtk_file_filter_add_pixbuf_formats(filter);
    gtk_file_chooser_add_filter(fc, filter);
}

void addFilterTex(GtkFileChooser* fc) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Latex files"));
#ifndef _WIN32
    gtk_file_filter_add_mime_type(filter, "application/x-latex");
    gtk_file_filter_add_mime_type(filter, "text/x-tex");
#else
    gtk_file_filter_add_pattern(filter, "*.tex");
#endif
    gtk_file_chooser_add_filter(fc, filter);
}
};  // namespace xoj
