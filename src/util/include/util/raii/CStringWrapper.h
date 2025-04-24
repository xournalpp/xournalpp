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
#include <utility>

#include <glib.h>

namespace xoj::util {
inline namespace raii {

template <class CharT = char>
class BasicOwnedCString final {
public:
    BasicOwnedCString() = default;
    BasicOwnedCString(const BasicOwnedCString&) = delete;
    BasicOwnedCString(BasicOwnedCString&& s): data_(std::exchange(s.data_, nullptr)){};
    BasicOwnedCString& operator=(const BasicOwnedCString&) = delete;
    BasicOwnedCString& operator=(BasicOwnedCString&& s) {
        g_free(data_);
        data_ = std::exchange(s.data_, nullptr);
        return *this;
    }
    ~BasicOwnedCString() { g_free(data_); }

    /**
     * @brief Assume ownership
     */
    template <class CharT2>
    static auto assumeOwnership(CharT2* s) -> BasicOwnedCString<CharT2> {
        BasicOwnedCString<CharT2> res;
        res.data_ = s;
        return res;
    }

    auto get() const -> CharT const* { return data_; }
    auto c_str() const -> CharT const* { return data_; }
    auto data() const -> CharT const* { return data_; }

    explicit operator bool() const { return data_ != nullptr && data_[0] != '\0'; }
    explicit operator std::basic_string_view<CharT>() const { return data_ ? data_ : std::string_view(); }

    const CharT& operator[](size_t n) const { return data_[n]; }

    /**
     * @brief Safely delete the content and return a pointer to the private data pointer
     *      Use to set the data with some C-libraries functions taking a char** as argument
     */
    CharT** contentReplacer() {
        g_free(std::exchange(data_, nullptr));
        return &data_;
    }

private:
    CharT* data_ = nullptr;
};

using OwnedCString = BasicOwnedCString<char>;

};  // namespace raii
};  // namespace xoj::util
