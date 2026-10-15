/*
 * Xournal++
 *
 * Helper functions to add filters to GtkFileChooserDialogs
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>

#include <gtk/gtk.h>

#include "util/i18n.h"

namespace xoj {
struct FileType {
    const char* extension;
    const char* mimetype;
    const char* description;
};

/// The filetypes we may save or export to, in SaveFileDialogs
namespace FileTypes {
// Use N_ for the descriptions so it is send to translators (but still constexpr)
constexpr FileType PDF = {"pdf", "application/pdf", N_("PDF files")};
constexpr FileType XOJ = {"xoj", "application/x-xojpp", N_("Xournal files")};
constexpr FileType XOPP = {"xopp", "application/x-xopp", N_("Xournal++ files")};
constexpr FileType XOPT = {"xopt", "application/x-xopt", N_("Xournal++ template")};
constexpr FileType SVG = {"svg", "image/svg+xml", N_("SVG graphics")};
constexpr FileType PNG = {"png", "image/png", N_("PNG graphics")};
constexpr FileType ZIP = {"zip", "application/zip", N_("ZIP archive")};
};  // namespace FileTypes
void addFilterForFile(GtkFileChooser* fc, const FileType& f);

inline void addFilterPdf(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::PDF); }
inline void addFilterXoj(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::XOJ); }
inline void addFilterXopp(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::XOPP); }
inline void addFilterXopt(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::XOPT); }
inline void addFilterSvg(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::SVG); }
inline void addFilterPng(GtkFileChooser* fc) { addFilterForFile(fc, FileTypes::PNG); }

void addFilterAllFiles(GtkFileChooser* fc);
void addFilterSupported(GtkFileChooser* fc);
void addFilterImages(GtkFileChooser* fc);  ///< All images supported by GdkPixbuf
void addFilterTex(GtkFileChooser* fc);
};  // namespace xoj
