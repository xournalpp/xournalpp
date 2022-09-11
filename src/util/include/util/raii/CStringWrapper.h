/*
 * Xournal++
 *
 * C-type strings wrapper, taking ownership of the data.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <string_view>

namespace xoj::util {
inline namespace raii {
class OwnedCString final {
public:
    OwnedCString() = default;
    OwnedCString(const OwnedCString&) = delete;
    OwnedCString(OwnedCString&& s);
    OwnedCString& operator=(const OwnedCString&) = delete;
    OwnedCString& operator=(OwnedCString&& s);
    ~OwnedCString();

    /**
     * @brief Assume ownership
     */
    static OwnedCString assumeOwnership(char* s);

    const char* get() const;
    operator bool() const;
    explicit operator std::string_view() const;

    /*
     * Why is this class implicitly convertible to const char& without this line?
     * In any case: this is dangerous, as it makes
     *   CString bar = "blob";
     *   std::string foo = bar;
     * compile without having the expected behaviour
     */
    operator const char&() const = delete;

    const char& operator[](size_t n) const;

    /**
     * @brief Safely delete the content and return a pointer to the private data pointer
     *      Use to set the data with some C-libraries functions taking a char** as argument
     */
    char** contentReplacer();

private:
    char* data = nullptr;
};
};  // namespace raii
};  // namespace xoj::util
