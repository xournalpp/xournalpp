/*
 * Xournal++
 *
 * An array indexed by a CONTIGUOUS enum class from 0 to _MAX_VALUE
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>
#include <type_traits>

#include "util/Assert.h"


template <class T, typename enum_class>
class EnumIndexedArray: private std::array<T, static_cast<size_t>(enum_class::_MAX_VALUE) + 1> {
public:
    static_assert(std::is_enum_v<enum_class>);
    using underlying_array_type = std::array<T, static_cast<size_t>(enum_class::_MAX_VALUE) + 1>;

    T& operator[](enum_class value) {
        xoj_assert(value <= enum_class::_MAX_VALUE);
        return underlying_array_type::operator[](static_cast<size_t>(value));
    }
    const T& operator[](enum_class value) const {
        xoj_assert(value <= enum_class::_MAX_VALUE);
        return underlying_array_type::operator[](static_cast<size_t>(value));
    }
    using underlying_array_type::begin;
    using underlying_array_type::end;
};
