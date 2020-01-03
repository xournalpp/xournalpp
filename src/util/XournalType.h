/*
 * Xournal++
 *
 * Type macros like GLib use for C to find wrong pointers at runtime
 *
 * The attributes start with z__ because if they start with __ they appear
 * as first element in the autocomplete list...
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <glib.h>

#include "logger/Logger.h"

#include "config-dev.h"


// Todo: unclean, remove in the future:

// Include string and vector everywhere
#include <cstring>  // Todo: Use std::string_view if possible
#include <string>
#include <vector>

using std::string;
using std::vector;
