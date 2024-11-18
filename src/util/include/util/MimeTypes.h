/*
 * Xournal++
 *
 * Structure for MimeTypes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <gtk/gtk.h>

namespace xoj {

struct MimeType {
    const char* const extension;
    const char* const mimeType;

    constexpr MimeType(const char* ext, const char* mime): extension(ext), mimeType(mime) {}

    void addToFilter(GtkFileFilter* filter) const {
#ifdef G_OS_WIN32
        gtk_file_filter_add_pattern(filter, ('*' + std::string(extension)).c_str());
#else
        gtk_file_filter_add_mime_type(filter, mimeType);
#endif
    }
};

// Predefined MimeTypes
namespace Mime {
constexpr MimeType PDF("pdf", "application/pdf");
constexpr MimeType SVG("svg", "image/svg+xml");
constexpr MimeType PNG("png", "image/png");

constexpr MimeType XOPT("xopt", "application/x-xopt");
constexpr MimeType XOJ("xoj", "application/x-xojpp");
constexpr MimeType XOPP("xopp", "application/x-xopp");
}  // namespace Mime
}  // namespace xoj