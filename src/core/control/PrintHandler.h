/*
 * Xournal++
 *
 * Prints a document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t

#include <gtk/gtk.h>  // for GtkWindow

class Document;

namespace PrintHandler {
void print(Document* doc, size_t currentPage, GtkWindow* parent);
}
