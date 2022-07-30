/*
 * Xournal++
 *
 * Identity function
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

namespace xoj::util {

inline namespace raii {
namespace specialization {
template <typename T>  // Todo(cpp20): use std:identity() and remove this header
constexpr auto identity = [](T* p) { return p; };
};
};  // namespace raii
};  // namespace xoj::util
