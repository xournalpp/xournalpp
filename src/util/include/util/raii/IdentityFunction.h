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

namespace std {

// Todo(cpp20): use std::identity() and remove this header
template <typename T>
constexpr auto identity = [](T* p) { return p; };

};  // namespace std
