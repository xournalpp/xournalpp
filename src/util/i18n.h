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

#include <boost/locale.hpp>

#define _(msg) boost::locale::translate(msg)
#define _C(msg) _(msg).str().c_str()
#define _F(msg) boost::locale::format(_(msg))
#define C_(context, msg) boost::locale::translate(context, msg)
#define C_C(context, msg) C_(context, msg).str().c_str()
#define C_F(context, msg) boost::locale::format(C_(context, msg))


/* Some helpler macros */

// boost::locale::format → std::string
#define FS(format) (format).str()
// boost::locale::format → char*
#define FC(format) FS(format).c_str()
