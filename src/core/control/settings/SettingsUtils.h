/*
 * Xournal++
 *
 * File containing utilities for the settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <array>
#include <map>
#include <string>

#include "model/Font.h"

#include "LatexSettings.h"
#include "SettingsDescription.h"
#include "SettingsImExporters.h"
#include "filesystem.h"

class SElement;

// Definitions of getter return types for value types
template <typename T>
struct getter_return {
    using type = T;
};
template <>
struct getter_return<std::string> {
    using type = const std::string&;
};
template <>
struct getter_return<fs::path> {
    using type = const fs::path&;
};
template <>
struct getter_return<XojFont> {
    using type = const XojFont&;
};
template <>
struct getter_return<LatexSettings> {
    using type = const LatexSettings&;
};
template <typename T, size_t N>
struct getter_return<std::array<T, N>> {
    using type = const std::array<T, N>&;
};
template <typename T, typename U>
struct getter_return<std::map<T, U>> {
    using type = const std::map<T, U>&;
};
template <>
struct getter_return<SElement> {
    using type = const SElement&;
};

template <typename T>
using getter_return_t = typename getter_return<T>::type;


// Importer, exporter, validator and comment struct here:
template <SettingsElement e, typename U = void>
struct importer {
    static constexpr auto fn = importProperty<typename Setting<e>::value_type>;
};
template <SettingsElement e>
struct importer<e, std::void_t<decltype(Setting<e>::IMPORT_FN)>> {
    static constexpr auto fn = Setting<e>::IMPORT_FN;
};

template <SettingsElement e, typename U = void>
struct exporter {
    static constexpr auto fn = exportProperty<getter_return_t<typename Setting<e>::value_type>>;
};
template <SettingsElement e>
struct exporter<e, std::void_t<decltype(Setting<e>::EXPORT_FN)>> {
    static constexpr auto fn = Setting<e>::EXPORT_FN;
};

template <SettingsElement e, typename U = void>
struct validator {
    static constexpr auto fn = [](const typename Setting<e>::value_type& val) ->
            typename Setting<e>::value_type { return val; };
    static constexpr bool enable = false;
};
template <SettingsElement e>
struct validator<e, std::void_t<decltype(Setting<e>::VALIDATE_FN)>> {
    static constexpr auto fn = Setting<e>::VALIDATE_FN;
    static constexpr bool enable = true;
};

template <SettingsElement e, typename U = void>
struct comment {
    static constexpr auto text = nullptr;
};
template <SettingsElement e>
struct comment<e, std::void_t<decltype(Setting<e>::COMMENT)>> {
    static constexpr auto text = Setting<e>::COMMENT;
};
