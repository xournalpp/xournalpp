/*
 * Xournal++
 *
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <bitset>
#include <string>
#include <type_traits>

#include <glib.h>

#include "util/Color.h"

namespace detail {
template <class T>
constexpr inline bool match_guint32() {
    return std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) == sizeof(guint32);
}
template <class T>
constexpr inline bool match_gint32() {
    return std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == sizeof(gint32);
}
template <class T>
constexpr inline bool match_guint64() {
    return std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) == sizeof(guint64);
}
template <class T>
constexpr inline bool match_gint64() {
    return std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == sizeof(gint64);
}

template <class T, class U = void>
struct GVariantWrapperImpl {};

template <>
struct GVariantWrapperImpl<bool> {
    static inline bool getValue(GVariant* v) { return g_variant_get_boolean(v); }
    static inline const GVariantType* getType() { return G_VARIANT_TYPE_BOOLEAN; }
    static inline GVariant* make(bool v) { return g_variant_new_boolean(v); }
};
template <>
struct GVariantWrapperImpl<const char*> {
    static inline const char* getValue(GVariant* v) { return g_variant_get_string(v, nullptr); }
    static inline const GVariantType* getType() { return G_VARIANT_TYPE_STRING; }
    static inline GVariant* make(const char* v) { return g_variant_new_string(v); }
};
template <>
struct GVariantWrapperImpl<double> {
    static inline double getValue(GVariant* v) { return g_variant_get_double(v); }
    static inline const GVariantType* getType() { return G_VARIANT_TYPE_DOUBLE; }
    static inline GVariant* make(double v) { return g_variant_new_double(v); }
};

template <class T>
struct GVariantWrapperImpl<T, std::enable_if_t<match_gint32<T>(), void>> {
    static inline T getValue(GVariant* v) { return g_variant_get_int32(v); }
    static inline const GVariantType* getType() { return G_VARIANT_TYPE_INT32; }
    static inline GVariant* make(T v) { return g_variant_new_int32(v); }
};
template <class T>
struct GVariantWrapperImpl<T, std::enable_if_t<match_guint32<T>(), void>> {
    static inline T getValue(GVariant* v) { return g_variant_get_uint32(v); }
    static inline const GVariantType* getType() { return G_VARIANT_TYPE_UINT32; }
    static inline GVariant* make(T v) { return g_variant_new_uint32(v); }
};
template <class T>
struct GVariantWrapperImpl<T, std::enable_if_t<match_gint64<T>(), void>> {
    static inline T getValue(GVariant* v) { return g_variant_get_int64(v); }
    static inline const GVariantType* getType() { return G_VARIANT_TYPE_INT64; }
    static inline GVariant* make(T v) { return g_variant_new_int64(v); }
};
template <class T>
struct GVariantWrapperImpl<T, std::enable_if_t<match_guint64<T>(), void>> {
    static inline T getValue(GVariant* v) { return g_variant_get_uint64(v); }
    static inline const GVariantType* getType() { return G_VARIANT_TYPE_UINT64; }
    static inline GVariant* make(T v) { return g_variant_new_uint64(v); }
};

template <class T>
struct GVariantWrapperImpl<T, std::enable_if_t<std::is_enum_v<T>, void>> {  // enum types are stored as guint64
    static inline T getValue(GVariant* v) { return static_cast<T>(g_variant_get_uint64(v)); }
    static inline const GVariantType* getType() { return G_VARIANT_TYPE_UINT64; }
    static inline GVariant* make(T v) { return g_variant_new_uint64(static_cast<guint64>(v)); }
};

template <>
struct GVariantWrapperImpl<Color> {
    static_assert(sizeof(guint32) == sizeof(Color));
    static inline Color getValue(GVariant* v) { return static_cast<Color>(g_variant_get_uint32(v)); }
    static inline const GVariantType* getType() { return G_VARIANT_TYPE_UINT32; }
    static inline GVariant* make(Color v) { return g_variant_new_uint32(static_cast<guint32>(v)); }
};
};  // namespace detail

template <class T>
inline T getGVariantValue(GVariant* v) {
    return detail::GVariantWrapperImpl<T>::getValue(v);
}
template <class T>
inline const GVariantType* gVariantType() {
    return detail::GVariantWrapperImpl<T>::getType();
}
template <class T>
inline GVariant* makeGVariant(T v) {
    return detail::GVariantWrapperImpl<T>::make(v);
}

template <>
inline GVariant* makeGVariant(const std::string& v) {
    return g_variant_new_string(v.c_str());
}
template <>
inline const GVariantType* gVariantType<const std::string&>() {
    return G_VARIANT_TYPE_STRING;
}
