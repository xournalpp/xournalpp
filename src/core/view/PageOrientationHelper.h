/*
 * Xournal++
 *
 * Helper function(s) for page orientation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_t

#include "model/XojPage.h"  // for PageOrientation

namespace xoj::view {
void cairoRotatePage(cairo_t* cr, PageOrientation pageOrient, double pageWidth, double pageHeight);
}
