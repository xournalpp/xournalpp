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

#include <gtk/gtk.h>

#include "XournalType.h"

class Document;

namespace PrintHandler {
void print(Document* doc, size_t currentPage, GtkWindow* parent);
}
