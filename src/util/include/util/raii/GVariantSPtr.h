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

#include <utility>

#include <glib-object.h>

#include "util/GVariantTemplate.h"

#include "CLibrariesSPtr.h"

namespace xoj::util {

inline namespace raii {
namespace specialization {

class GVariantHandler {
public:
    constexpr static auto ref = [](GVariant* v) { return g_variant_ref(v); };
    constexpr static auto unref = [](GVariant* v) { g_variant_unref(v); };
    constexpr static auto ref_sink = [](GVariant* v) { return g_variant_ref_sink(v); };
    constexpr static auto adopt = [](GVariant* v) { return g_variant_take_ref(v); };
};
};  // namespace specialization

using GVariantSPtr = CLibrariesSPtr<GVariant, raii::specialization::GVariantHandler>;

template <typename T>
GVariantSPtr makeGVariantSPtr(T t) {
    return GVariantSPtr(makeGVariant(t), adopt);
}

};  // namespace raii
};  // namespace xoj::util
