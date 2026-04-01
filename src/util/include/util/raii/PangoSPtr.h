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

#include <pango/pango-attributes.h>  // for PangoAttrList, pango_attr_list_ref, pango_attr_list_unref

#include "CLibrariesSPtr.h"
#include "IdentityFunction.h"

namespace xoj::util {
inline namespace raii {

namespace specialization {
class PangoAttrListHandler {
public:
    constexpr static auto ref = pango_attr_list_ref;
    constexpr static auto unref = pango_attr_list_unref;
    constexpr static auto adopt = identity<PangoAttrList>;
};
};  // namespace specialization

using PangoAttrListSPtr = CLibrariesSPtr<PangoAttrList, raii::specialization::PangoAttrListHandler>;
};  // namespace raii
};  // namespace xoj::util
