// UTF-8 string view
#pragma once

#include <iterator>
#include <ranges>
#include <type_traits>

template <std::input_iterator InputIterator, std::sentinel_for<InputIterator> InputSentinel>
struct utf8_view: std::ranges::view_interface<utf8_view<InputIterator, InputSentinel>> {
    struct SentinelImpl {
        constexpr SentinelImpl() = default;
        constexpr SentinelImpl(InputSentinel it): it(it) {}

        InputSentinel it{};
    };

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = char8_t;
        using difference_type = std::iterator_traits<InputIterator>::difference_type;
        using pointer = char8_t;
        using reference = char8_t;

        constexpr Iterator() = default;
        constexpr Iterator(InputIterator it): it(it) {}

        constexpr auto operator*() const -> reference { return *it; }

        constexpr auto operator++() -> Iterator& {
            ++it;
            return *this;
        }

        constexpr auto operator++(int) -> Iterator {
            auto copy = *this;
            ++it;
            return copy;
        }

        constexpr auto operator==(Iterator const& other) const -> bool { return it == other.it; }

        // constexpr auto operator!=(Iterator const& other) const -> bool { return it != other.it; }

        friend constexpr auto operator==(Iterator const& lhs, SentinelImpl const& other) -> bool {
            return lhs.it == other.it;
        }

        // friend constexpr auto operator!=(Iterator const& lhs, SentinelImpl const& other) -> bool {
        //     return lhs.it != other.it;
        // }

        InputIterator it{};
    };


    using Sentinel = std::conditional_t<std::is_same_v<InputSentinel, InputIterator>, Iterator, SentinelImpl>;

    constexpr utf8_view(InputIterator beginI, InputSentinel endI): beginI(beginI), endI(endI) {}

    constexpr auto begin() const -> Iterator { return beginI; }
    constexpr auto end() const -> Sentinel { return endI; }

    auto str() const -> std::u8string { return std::u8string{beginI, endI}; }

private:
    Iterator beginI;
    Sentinel endI;
};

struct CharSentinelClass {
    constexpr auto operator==(char const* v) const -> bool { return *v == '\0'; }
};
constexpr inline CharSentinelClass char_sentinel{};
static_assert(std::sentinel_for<CharSentinelClass, char const*>);
static_assert(std::sentinel_for<utf8_view<char const*, CharSentinelClass>::Sentinel,
                                utf8_view<char const*, CharSentinelClass>::Iterator>);

// RangeAdaptorClosureObject for utf8_view
struct utf8_t {

    template <std::ranges::viewable_range R>
    auto operator()(R const& r) const {
        return utf8_view{std::begin(r), std::end(r)};
    }

    auto operator()(char const* r) const { return utf8_view{r, char_sentinel}; }
};

inline constexpr utf8_t utf8;

template <std::ranges::viewable_range R>
auto operator|(R r, utf8_t const&) {
    return utf8_view{std::begin(r), std::end(r)};
}

inline auto operator|(char const* r, utf8_t const&) { return utf8_view{r, char_sentinel}; }

// template<class InI, class InS>
// constexpr auto begin(utf8_view<InI, InS> const& view) -> utf8_view<InI, InS>::Iterator { return view.begin(); }

// template<class InI, class InS>
// constexpr auto end(utf8_view<InI, InS> const& view) -> utf8_view<InI, InS>::Sentinel { return view.end(); }

constexpr auto* s = "Hello, world!";
constexpr std::string_view sv = "Hello, world!";

constexpr utf8_view<char const*, char const*> view{sv.data(), sv.data() + sv.size()};
static_assert(*std::begin(utf8_view(std::begin(sv), sv.end())) == char8_t(*std::begin(sv)));
static_assert(std::forward_iterator<std::string::iterator>);
static_assert(std::forward_iterator<char const*>);
static_assert(std::forward_iterator<utf8_view<std::string::iterator, std::string::iterator>::Iterator>);
static_assert(std::forward_iterator<utf8_view<char*, char*>::Iterator>);
static_assert(std::ranges::range<utf8_view<std::string::iterator, std::string::iterator>>);
static_assert(std::ranges::view<utf8_view<std::string::iterator, std::string::iterator>>);
static_assert(std::ranges::view<utf8_view<char const*, char const*>>);

static_assert(std::ranges::viewable_range<decltype(sv | utf8)>);
static_assert(std::ranges::viewable_range<decltype(utf8(s))>);
static_assert(std::ranges::viewable_range<decltype(s | utf8)>);