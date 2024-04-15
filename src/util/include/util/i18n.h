/*
 * Xournal++
 *
 * Internationalization module
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <libintl.h>

#include "util/PlaceholderString.h"  // IWYU pragma: keep
#include "util/utf8_view.h"          // IWYU pragma: keep

#undef snprintf

#define _(msg) (gettext(msg))
#define _utf8(msg) (gettext(msg) | utf8)
#define C_(context, msg) g_dpgettext2(nullptr, context, msg)

/// The string is not looked for by xgettext and should be added to the .po files another way (e.g. with N_ below)
#define fetch_translation(msg) gettext(msg)
#define fetch_translation_context(context, msg) g_dpgettext2(nullptr, context, msg)

// Formatted Translation
#define _F(msg) makePlaceholderString(_(msg))
#define C_F(context, msg) makePlaceholderString(C_(context, msg))

// Formatted, not translated text
#define FORMAT_STR(msg) makePlaceholderString(msg)


// No translation performed, but in the Translation string
// So translation can be loaded dynamically at other place
// in the code
#define N_(msg) (msg)
#define NC_(context, msg) (msg)

/* Some helper macros */

// PlaceholderString → std::string
#define FS(format) (format).str()
// PlaceholderString → const char*
#define FC(format) FS(format).c_str()
