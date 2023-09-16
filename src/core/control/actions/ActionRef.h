/*
 * Xournal++
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <gio/gio.h>

#include "util/raii/GObjectSPtr.h"

using ActionRef = xoj::util::GObjectSPtr<GSimpleAction>;
