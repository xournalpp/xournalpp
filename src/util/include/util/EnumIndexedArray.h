/*
 * Xournal++
 *
 * An array indexed by a CONTIGUOUS scoped enum from 0 to ENUMERATOR_COUNT-1
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
          std::enable_if_t<std::is_enum_v<enum_class> && std::is_unsigned_v<std::underlying_type_t<enum_class>> &&
                                   !std::is_convertible_v<enum_class, std::underlying_type_t<enum_class>>,
                           bool> = true>
class EnumIndexedArray: private std::array<T, xoj::to_underlying(enum_class::ENUMERATOR_COUNT)> {
public:
    using underlying_array_type = std::array<T, xoj::to_underlying(enum_class::ENUMERATOR_COUNT)>;

    template <typename... Args>
    constexpr EnumIndexedArray(Args&&... args): underlying_array_type{std::forward<Args>(args)...} {};

    T& operator[](enum_class value) {
        xoj_assert(value < enum_class::ENUMERATOR_COUNT);
        return underlying_array_type::operator[](xoj::to_underlying(value));
    }
    const T& operator[](enum_class value) const {
        xoj_assert(value < enum_class::ENUMERATOR_COUNT);
        return underlying_array_type::operator[](xoj::to_underlying(value));
    }
    using underlying_array_type::begin;
    using underlying_array_type::end;
};
