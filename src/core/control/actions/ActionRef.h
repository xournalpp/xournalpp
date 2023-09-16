/**
 *
 *
 */
#pragma once

#include <gio/gio.h>

#include "util/raii/GObjectSPtr.h"

using ActionRef = xoj::util::GObjectSPtr<GSimpleAction>;
