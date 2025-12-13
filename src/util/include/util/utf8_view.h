// UTF-8 string view
#pragma once

#include <iterator>
#include <ranges>
#include <type_traits>

#include "util/ViewIteratorBase.h"

#include "filesystem.h"

namespace xoj::util {
template <std::input_iterator InputIterator, std::sentinel_for<InputIterator> InputSentinel>
struct utf8_view: std::ranges::view_interface<utf8_view<InputIterator, InputSentinel>> {
    struct SentinelImpl {
        constexpr SentinelImpl() = default;
        constexpr SentinelImpl(InputSentinel it): it(it) {}

        InputSentinel it{};
    };

    struct Iterator: public ViewIteratorBase<Iterator, InputIterator> {
        using value_type = char8_t;
        using pointer = char8_t;
        using reference = char8_t;

        constexpr Iterator() = default;
        constexpr Iterator(InputIterator it): ViewIteratorBase<Iterator, InputIterator>(it) {}

        friend constexpr auto operator==(Iterator const& lhs, SentinelImpl const& other) -> bool {
            return lhs.it == other.it;
        }

        constexpr auto operator*() const -> reference {
            return static_cast<reference>(*ViewIteratorBase<Iterator, InputIterator>::it);
        }
        constexpr auto operator[](size_t n) const -> reference {
            return static_cast<reference>(ViewIteratorBase<Iterator, InputIterator>::it[n]);
        }
    };


    using Sentinel = std::conditional_t<std::is_same_v<InputSentinel, InputIterator>, Iterator, SentinelImpl>;

    constexpr utf8_view(InputIterator beginI, InputSentinel endI): beginI(beginI), endI(endI) {}

    constexpr auto begin() const -> Iterator { return beginI; }
    constexpr auto end() const -> Sentinel { return endI; }


    constexpr auto str() const -> std::u8string {
        // c++20 has no string constructor using iterator and sentinel.
        // todo(cpp23) use std::u8string(std::from_range_t, ...)
        auto [b, e] = toIteratorPair();
        return std::u8string(b, e);
    }

    explicit operator fs::path() const {
        // c++ has no fs::path constructor using iterator and sentinel.
        auto [b, e] = toIteratorPair();
        return fs::path(b, e);
    }

    bool operator==(std::u8string_view other) const { return std::ranges::equal(*this, other); }

private:
    constexpr auto toIteratorPair() const -> std::pair<Iterator, Iterator> {
        if constexpr (std::is_same_v<InputIterator, InputSentinel>) {
            return std::pair{beginI, std::next(beginI, std::distance(beginI.it, endI.it))};
        } else {
            auto it = beginI;
            while (it != endI) {
                ++it;
            }
            return std::pair{beginI, it};
        }
    }

    Iterator beginI;
    Sentinel endI;
};

template <typename T>
concept is_byte = sizeof(T) == sizeof(char8_t) && std::convertible_to<T, char8_t>;

template <is_byte T>
struct CharSentinelClass {
    constexpr auto operator==(T const* v) const -> bool { return *v == T('\0'); }
};
static_assert(std::sentinel_for<CharSentinelClass<char>, char const*>);
static_assert(std::sentinel_for<utf8_view<char const*, CharSentinelClass<char>>::Sentinel,
                                utf8_view<char const*, CharSentinelClass<char>>::Iterator>);
static_assert(std::sentinel_for<CharSentinelClass<unsigned char>, unsigned char const*>);
static_assert(std::sentinel_for<utf8_view<unsigned char const*, CharSentinelClass<unsigned char>>::Sentinel,
                                utf8_view<unsigned char const*, CharSentinelClass<unsigned char>>::Iterator>);

// RangeAdaptorClosureObject for utf8_view
struct utf8_t {
    template <std::ranges::viewable_range R>
    auto operator()(R const& r) const {
        return utf8_view{std::begin(r), std::end(r)};
    }

    // For some reason, gcc 11 (default on ubuntu 22 LTS) does not recognize std::string as a viewable_range
    template <is_byte T>
    auto operator()(std::basic_string<T> const& r) const {
        return utf8_view{std::begin(r), std::end(r)};
    }

    template <is_byte T>
    auto operator()(T const* r) const {
        return utf8_view{r, CharSentinelClass<T>{}};
    }
};

inline constexpr utf8_t utf8;

template <std::ranges::viewable_range R>
constexpr auto operator|(R r, utf8_t const&) {
    return utf8_view{std::begin(r), std::end(r)};
}

template <is_byte T>
inline constexpr auto operator|(T const* r, utf8_t const&) {
    return utf8_view{r, CharSentinelClass<T>{}};
}

// template<class InI, class InS>
// constexpr auto begin(utf8_view<InI, InS> const& view) -> utf8_view<InI, InS>::Iterator { return view.begin(); }

// template<class InI, class InS>
// constexpr auto end(utf8_view<InI, InS> const& view) -> utf8_view<InI, InS>::Sentinel { return view.end(); }

namespace {
constexpr auto* s = "Hello, world!";
constexpr std::string_view sv = "Hello, world!";
constexpr unsigned char us[] = {0xE2, 0x84, 0x8F,  // ℏ
                                0xE2, 0x93, 0x94,  // ⓔ
                                0xE2, 0x84, 0x93,  // ℓ
                                0x6C,              // l
                                0xC6, 0xA1,        // ơ
                                0x00};

constexpr utf8_view<char const*, char const*> view{sv.data(), sv.data() + sv.size()};

static_assert(*std::begin(utf8_view(std::begin(sv), sv.end())) == char8_t(*std::begin(sv)));
static_assert(*std::begin(utf8_view(std::begin(sv), sv.end())) == u8'H');
static_assert(*(utf8_view<char const*, char const*>{sv.data(), sv.data() + sv.size()}.begin()) == u8'H');
static_assert(std::random_access_iterator<std::string::iterator>);
static_assert(std::random_access_iterator<char const*>);
static_assert(std::random_access_iterator<unsigned char const*>);
static_assert(std::random_access_iterator<utf8_view<std::string::iterator, std::string::iterator>::Iterator>);
static_assert(std::random_access_iterator<utf8_view<char*, char*>::Iterator>);
static_assert(std::ranges::random_access_range<utf8_view<std::string::iterator, std::string::iterator>>);
static_assert(std::ranges::view<utf8_view<std::string::iterator, std::string::iterator>>);
static_assert(std::ranges::view<utf8_view<char const*, char const*>>);

static_assert(std::ranges::viewable_range<decltype(sv | utf8)>);
static_assert(std::ranges::viewable_range<decltype(utf8(s))>);
static_assert(std::ranges::viewable_range<decltype(s | utf8)>);
static_assert(std::ranges::random_access_range<decltype(sv | utf8)>);
static_assert(std::ranges::random_access_range<decltype(utf8(s))>);
static_assert(std::ranges::random_access_range<decltype(s | utf8)>);
static_assert(std::ranges::random_access_range<decltype(us | utf8)>);

#if __cpp_lib_constexpr_string == 201907L
static_assert(utf8_view<char const*, char const*>{sv.data(), sv.data() + sv.size()}.str() == u8"Hello, world!");
static_assert(utf8_view{sv.data(), CharSentinelClass<char>{}}.str() == u8"Hello, world!");
static_assert((us | utf8).str() == u8"ℏⓔℓlơ");
#endif
};  // namespace
};  // namespace xoj::util
