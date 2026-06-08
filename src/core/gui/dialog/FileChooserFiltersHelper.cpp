#include "FileChooserFiltersHelper.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "util/i18n.h"

#include "NativeFileChooserHelper.h"

namespace xoj {
namespace {
void addExtensionPattern(GtkFileFilter* filter, const char* extension) {
    std::string lowerPattern = "*";
    lowerPattern += extension;
    gtk_file_filter_add_pattern(filter, lowerPattern.c_str());

    std::string upperExt = extension;
    std::transform(upperExt.begin(), upperExt.end(), upperExt.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    if (upperExt != extension) {
        std::string upperPattern = "*";
        upperPattern += upperExt;
        gtk_file_filter_add_pattern(filter, upperPattern.c_str());
    }
}
}  // namespace

bool useNativeFileChooser() { return xoj::NativeFileChooser::isAvailable(); }

static void addMimeTypeFilter(GtkFileChooser* fc, const char* name, const char* mime) {
    GtkFileFilter* filterPdf = gtk_file_filter_new();
    gtk_file_filter_set_name(filterPdf, name);
    gtk_file_filter_add_mime_type(filterPdf, mime);
    gtk_file_chooser_add_filter(fc, filterPdf);
}

void addFilterByExtension(GtkFileChooser* fc, const char* name, std::initializer_list<const char*> extensions) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, name);
    for (const char* extension: extensions) {
        addExtensionPattern(filter, extension);
    }
    gtk_file_chooser_add_filter(fc, filter);
}

void addFilterAllFiles(GtkFileChooser* fc) {
    GtkFileFilter* filterAll = gtk_file_filter_new();
    gtk_file_filter_set_name(filterAll, _("All files"));
    gtk_file_filter_add_pattern(filterAll, "*");
    gtk_file_chooser_add_filter(fc, filterAll);
}

void addFilterSupportedByExtension(GtkFileChooser* fc) {
    addFilterByExtension(fc, _("Supported files"), {".xopp", ".xoj", ".xopt", ".pdf", ".moj"});
}
void addFilterPdfByExtension(GtkFileChooser* fc) { addFilterByExtension(fc, _("PDF files"), {".pdf"}); }
void addFilterXojByExtension(GtkFileChooser* fc) { addFilterByExtension(fc, _("Xournal files"), {".xoj"}); }
void addFilterXoppByExtension(GtkFileChooser* fc) { addFilterByExtension(fc, _("Xournal++ files"), {".xopp"}); }
void addFilterXoptByExtension(GtkFileChooser* fc) { addFilterByExtension(fc, _("Xournal++ template"), {".xopt"}); }
void addFilterImagesByExtension(GtkFileChooser* fc) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Image files"));

    GSList* formats = gdk_pixbuf_get_formats();
    for (GSList* iter = formats; iter != nullptr; iter = iter->next) {
        auto* format = static_cast<GdkPixbufFormat*>(iter->data);
        char** extensions = gdk_pixbuf_format_get_extensions(format);
        for (char** extension = extensions; extension && *extension; extension++) {
            std::string patternExtension = ".";
            patternExtension += *extension;
            addExtensionPattern(filter, patternExtension.c_str());
        }
        g_strfreev(extensions);
    }
    g_slist_free(formats);

    gtk_file_chooser_add_filter(fc, filter);
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

void addFilterImages(GtkFileChooser* fc) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Image files"));
    gtk_file_filter_add_pixbuf_formats(filter);
    gtk_file_chooser_add_filter(fc, filter);
}
};  // namespace xoj
