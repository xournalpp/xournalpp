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

#include <gtk/gtk.h>

namespace xoj {
void addFilterAllFiles(GtkFileChooser* fc);
void addFilterSupported(GtkFileChooser* fc);
void addFilterPdf(GtkFileChooser* fc);
void addFilterXoj(GtkFileChooser* fc);
void addFilterXopp(GtkFileChooser* fc);
void addFilterXopt(GtkFileChooser* fc);
void addFilterSvg(GtkFileChooser* fc);
void addFilterPng(GtkFileChooser* fc);
void addFilterImages(GtkFileChooser* fc);  ///< All images supported by GdkPixbuf
void addFilterTex(GtkFileChooser* fc);
};  // namespace xoj
