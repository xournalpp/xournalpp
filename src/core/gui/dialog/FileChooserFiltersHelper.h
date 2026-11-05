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

#include <initializer_list>

#include <gtk/gtk.h>

namespace xoj {
bool useNativeFileChooser();
void addFilterByExtension(GtkFileChooser* fc, const char* name, std::initializer_list<const char*> extensions);
void addFilterAllFiles(GtkFileChooser* fc);
void addFilterSupportedByExtension(GtkFileChooser* fc);
void addFilterPdfByExtension(GtkFileChooser* fc);
void addFilterXojByExtension(GtkFileChooser* fc);
void addFilterXoppByExtension(GtkFileChooser* fc);
void addFilterXoptByExtension(GtkFileChooser* fc);
void addFilterImagesByExtension(GtkFileChooser* fc);
void addFilterSupported(GtkFileChooser* fc);
void addFilterPdf(GtkFileChooser* fc);
void addFilterXoj(GtkFileChooser* fc);
void addFilterXopp(GtkFileChooser* fc);
void addFilterXopt(GtkFileChooser* fc);
void addFilterSvg(GtkFileChooser* fc);
void addFilterPng(GtkFileChooser* fc);
void addFilterImages(GtkFileChooser* fc);  ///< All images supported by GdkPixbuf
};  // namespace xoj
