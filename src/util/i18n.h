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
#define C_(context, msg) boost::locale::translate(context, msg)
#define C_C(context, msg) C_(context, msg).str().c_str()
