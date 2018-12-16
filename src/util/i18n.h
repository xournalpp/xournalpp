/*
 * Xournal++
 *
 * Internationalization module
 *
 * @author MarPiRK
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <libintl.h>

#include <boost/locale.hpp>

#define _(msg) boost::locale::translate(msg)
// Use standard gettext, where the string is const and no issue with freed strings
#define _C(msg) gettext(msg)
#define _F(msg) boost::locale::format(_(msg))
#define C_(context, msg) boost::locale::translate(context, msg)
// Use standard gettext, where the string is const and no issue with freed strings
#define C_C(context, msg) g_dpgettext (NULL, context "\004" msg, strlen(context) + 1)
#define C_F(context, msg) boost::locale::format(C_(context, msg))


/* Some helper macros */

// boost::locale::format → std::string
#define FS(format) (format).str()
// boost::locale::format → char*
#define FC(format) FS(format).c_str()
