/*
 * Xournal++
 *
 * Internationalization module
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

// Todo(fabian): replace with cpp20 format library or https://github.com/fmtlib/fmt which are the same
//               also using python like format string syntax

#pragma once

#include <cstddef>  // for size_t
#include <cstdint>  // for int64_t
#include <memory>   // for unique_ptr
#include <ostream>  // for ostream
#include <string>   // for string
#include <string_view>
#include <type_traits>
#include <utility>  // for move
#include <vector>   // for vector

#include <glib.h>  // for g_error

#include "util/StringUtils.h"

template <typename C>
concept Character = std::is_same_v<char, std::remove_cv_t<C>> ||      //
                    std::is_same_v<wchar_t, std::remove_cv_t<C>> ||   //
                    std::is_same_v<char8_t, std::remove_cv_t<C>> ||   //
                    std::is_same_v<char16_t, std::remove_cv_t<C>> ||  //
                    std::is_same_v<char32_t, std::remove_cv_t<C>>;

template <typename StringType>
concept StringLike = requires(StringType s) {
    { s.c_str() };
    { s.size() };
};


/**
 * Base class for Formatting
 */
class PlaceholderElement {
public:
    virtual ~PlaceholderElement() = default;
    PlaceholderElement() = default;
    PlaceholderElement(PlaceholderElement const& p) = default;
    PlaceholderElement(PlaceholderElement&& p) = default;
    PlaceholderElement& operator=(PlaceholderElement const&) = default;
    PlaceholderElement& operator=(PlaceholderElement&&) = default;
    virtual auto format(std::string_view format) const -> std::string = 0;
};


/**
 * Format String
 */
class PlaceholderElementString: public PlaceholderElement {
public:
    explicit PlaceholderElementString(std::string_view text): text(text) {}

    auto format(std::string_view format) const -> std::string override { return {text.begin(), text.end()}; }

private:
    std::string_view text;
};

/**
 * Format int
 */
template <typename T>
class PlaceholderElementInt: public PlaceholderElement {
public:
    explicit PlaceholderElementInt(T value): value(value) {}

    auto format(std::string_view format) const -> std::string override { return std::to_string(value); }

private:
    T value;
};


/**
 * Placeholder String, used for formatting. Support Placeholder like
 * {1}, {2} etc. Use {{ for {
 */
struct PlaceholderString {

    explicit PlaceholderString(std::string_view text);

    // Placeholder methods
    template <typename T>
    auto operator%(T const& value) -> PlaceholderString& {
        if constexpr (std::is_integral_v<T>) {
            data.emplace_back(std::make_unique<PlaceholderElementInt<T>>(value));
        } else if constexpr (std::is_same_v<std::u8string, T> || std::is_same_v<std::u8string_view, T>) {
            data.emplace_back(std::make_unique<PlaceholderElementString>(char_cast(value)));
        } else if constexpr (std::is_same_v<char const&, T>) {
            data.emplace_back(std::make_unique<PlaceholderElementString>(std::string_view(&value, 1)));
        } else {
            data.emplace_back(std::make_unique<PlaceholderElementString>(value));
        }
        return *this;
    }

    // Process Method
    auto str() const -> std::string;
    auto c_str() const -> char const*;  // NOLINT(readability-identifier-naming)

private:
    auto formatPart(std::string_view format) const -> std::string;
    void process() const;

    /**
     * Values for Placeholder
     */
    std::vector<std::unique_ptr<PlaceholderElement>> data;

    /**
     * Input text
     */
    std::string_view text;

    /**
     * Processed String
     */
    mutable std::string processed;
};

template <class String>
static auto makePlaceholderString(String&& val) {
    return PlaceholderString(std::forward<String>(val));
}

auto operator<<(std::ostream& os, PlaceholderString& ps) -> std::ostream&;
