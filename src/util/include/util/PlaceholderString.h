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
#include <utility>  // for move
#include <vector>   // for vector

#include <glib.h>  // for g_error

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
    virtual auto format(std::string format) const -> std::string = 0;
};


/**
 * Format String
 */
class PlaceholderElementString: public PlaceholderElement {
public:
    explicit PlaceholderElementString(std::string_view text): text(text) {}

    auto format(std::string format) const -> std::string override { return text; }

private:
    std::string text;
};

/**
 * Format int
 */
template <typename T>
class PlaceholderElementInt: public PlaceholderElement {
public:
    explicit PlaceholderElementInt(T value): value(value) {}

    auto format(std::string format) const -> std::string override { return std::to_string(value); }

private:
    T value;
};


/**
 * Placeholder String, used for formatting. Support Placeholder like
 * {1}, {2} etc. Use {{ for {
 */
struct PlaceholderString {
    PlaceholderString(std::string text);

    // Placeholder methods
    template <typename T>
    auto operator%(T value) -> PlaceholderString& {
        if constexpr (std::is_integral_v<T>) {
            data.emplace_back(std::make_unique<PlaceholderElementInt<T>>(value));
        } else {
            data.emplace_back(std::make_unique<PlaceholderElementString>(std::move(value)));
        }
        return *this;
    }

    // Process Method
    std::string str() const;
    const char* c_str() const;  // NOLINT(readability-identifier-naming)

private:
    std::string formatPart(std::string format) const;
    void process() const;

    /**
     * Values for Placeholder
     */
    std::vector<std::unique_ptr<PlaceholderElement>> data;

    /**
     * Input text
     */
    std::string text;

    /**
     * Processed String
     */
    mutable std::string processed;
};

std::ostream& operator<<(std::ostream& os, PlaceholderString& ps);
