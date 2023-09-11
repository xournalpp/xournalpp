/*
 * Xournal++
 *
 * An array indexed by a CONTIGUOUS enum class from 0 to _COUNT-1
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
#include "util/safe_casts.h"  // for to_underlying Todo(cpp20) use <utility>


template <class T, typename enum_class,
          std::enable_if_t<std::is_enum_v<enum_class> && std::is_unsigned_v<std::underlying_type_t<enum_class>>, bool> =
                  true>
class EnumIndexedArray: private std::array<T, std::to_underlying(enum_class::_COUNT)> {
public:
    using underlying_array_type = std::array<T, std::to_underlying(enum_class::_COUNT)>;

    T& operator[](enum_class value) {
        xoj_assert(value < enum_class::_COUNT);
        return underlying_array_type::operator[](std::to_underlying(value));
    }
    const T& operator[](enum_class value) const {
        xoj_assert(value < enum_class::_COUNT);
        return underlying_array_type::operator[](std::to_underlying(value));
    }
    using underlying_array_type::begin;
    using underlying_array_type::end;
};
