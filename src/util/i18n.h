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

#include "PlaceholderString.h"

#include <libintl.h>
#undef snprintf

#define _(msg) gettext(msg)
#define C_(context, msg) g_dpgettext2 (nullptr, context, msg)

// Formatted Translation
#define _F(msg) PlaceholderString(_(msg))
#define C_F(context, msg) PlaceholderString(C_(context, msg))

// Formatted, not translated text
#define FORMAT_STR(msg) PlaceholderString(msg)


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
