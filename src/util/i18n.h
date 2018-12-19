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

#define _(msg) gettext(msg)
#define C_(context, msg) g_dpgettext (NULL, context "\004" msg, strlen(msg) + 1)

#define _F(msg) boost::locale::format(_(msg))
#define C_F(context, msg) boost::locale::format(C_(context, msg))


/* Some helper macros */

// boost::locale::format → std::string
#define FS(format) (format).str()
// boost::locale::format → char*
#define FC(format) FS(format).c_str()
