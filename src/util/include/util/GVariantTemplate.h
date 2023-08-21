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

template <class T>
inline T getGVariantValue(GVariant* v) {
    static_assert(std::is_enum_v<T>);  // enum types are stored as size_t
    return static_cast<T>(g_variant_get_uint64(v));
}

template <>
inline bool getGVariantValue<bool>(GVariant* v) {
    return g_variant_get_boolean(v);
}
template <>
inline uint32_t getGVariantValue<uint32_t>(GVariant* v) {
    return g_variant_get_uint32(v);
}
template <>
inline size_t getGVariantValue<size_t>(GVariant* v) {
    return g_variant_get_uint64(v);
}
template <>
inline int getGVariantValue<int>(GVariant* v) {
    return g_variant_get_int32(v);
}
template <>
inline const char* getGVariantValue<const char*>(GVariant* v) {
    return g_variant_get_string(v, nullptr);
}
template <>
inline double getGVariantValue<double>(GVariant* v) {
    return g_variant_get_double(v);
}

template <class T>
inline GVariant* makeGVariant(T v) {
    static_assert(std::is_enum_v<T>);  // enum types are stored as size_t
    return g_variant_new_uint64(static_cast<size_t>(v));
}

template <>
inline GVariant* makeGVariant(bool v) {
    return g_variant_new_boolean(v);
}
template <>
inline GVariant* makeGVariant(uint32_t v) {
    return g_variant_new_uint32(v);
}
template <>
inline GVariant* makeGVariant(size_t v) {
    return g_variant_new_uint64(v);
}
template <>
inline GVariant* makeGVariant(int v) {
    return g_variant_new_int32(v);
}
template <>
inline GVariant* makeGVariant(const char* v) {
    return g_variant_new_string(v);
}
template <>
inline GVariant* makeGVariant(const std::string& v) {
    return g_variant_new_string(v.c_str());
}
template <>
inline GVariant* makeGVariant(double v) {
    return g_variant_new_double(v);
}

template <class T>
inline constexpr const GVariantType* gVariantType() {
    static_assert(std::is_enum_v<T>);  // enum types are stored as size_t
    return G_VARIANT_TYPE_UINT64;
}

template <>
inline const GVariantType* gVariantType<void>() {
    return nullptr;
}

template <>
inline const GVariantType* gVariantType<bool>() {
    return G_VARIANT_TYPE_BOOLEAN;
}
template <>
inline const GVariantType* gVariantType<uint32_t>() {
    return G_VARIANT_TYPE_UINT32;
}
template <>
inline const GVariantType* gVariantType<size_t>() {
    return G_VARIANT_TYPE_UINT64;
}
template <>
inline const GVariantType* gVariantType<int>() {
    return G_VARIANT_TYPE_INT32;
}
template <>
inline const GVariantType* gVariantType<const char*>() {
    return G_VARIANT_TYPE_STRING;
}
template <>
inline const GVariantType* gVariantType<const std::string&>() {
    return G_VARIANT_TYPE_STRING;
}
template <>
inline const GVariantType* gVariantType<double>() {
    return G_VARIANT_TYPE_DOUBLE;
}
