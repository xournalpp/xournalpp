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

#include <cstdint>    // for int64_t
#include <memory>     // for unique_ptr
#include <ostream>    // for ostream
#include <string>     // for string
#include <vector>     // for vector

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
 * Placeholder String, used for formatting. Support Placeholder like
 * {1}, {2} etc. Use {{ for {
 */
struct PlaceholderString {
    PlaceholderString(std::string text);

    // Placeholder methods
    PlaceholderString& operator%(int64_t value);
    PlaceholderString& operator%(std::string value);

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
