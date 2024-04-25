/*
 * Xournal++
 *
 * RAII wrappers for C library classes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <glib/gbytes.h>

#include "CLibrariesSPtr.h"
#include "IdentityFunction.h"

namespace xoj::util {
inline namespace raii {

namespace specialization {
class GBytesHandler {
public:
    constexpr static auto ref = [](GBytes* p) { return g_bytes_ref(p); };
    constexpr static auto unref = [](GBytes* p) { g_bytes_unref(p); };
    constexpr static auto adopt = identity<GBytes>;
};
};  // namespace specialization

using GBytesSPtr = CLibrariesSPtr<GBytes, raii::specialization::GBytesHandler>;
};  // namespace raii
};  // namespace xoj::util
