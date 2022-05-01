#pragma once

#include <memory>
#include <sstream>
#include <string>

namespace xoj::util::log {

auto printBugreportNotice(std::ostream& stream) -> std::ostream&;

class DumplogPrivate;

class Dumplog {
public:
    // Used as array index
    enum Type { ERRORLOG = 0, DEADLOG = 1 };

    enum PredefinedHeaderFlags { HEADER_TIME = 0x01 };

    Dumplog(Type type, const char* msgPrefix, PredefinedHeaderFlags header);
    Dumplog(const Dumplog&) = delete;
    Dumplog(Dumplog&& d);
    ~Dumplog();

    auto operator=(const Dumplog&) -> Dumplog& = delete;
    auto operator=(Dumplog&&) -> Dumplog& = default;

    auto stream() -> std::ostream&;
    auto str() const -> std::string;

    void addHeaderLine(std::string l);

    template <class T>
    auto operator<<(T&& o) -> Dumplog& {
        stream() << std::forward<T>(o);
        return *this;
    }

private:
    std::unique_ptr<DumplogPrivate> impl_;
};

inline auto operator|(Dumplog::PredefinedHeaderFlags lhs, Dumplog::PredefinedHeaderFlags rhs)
        -> Dumplog::PredefinedHeaderFlags {

    using Flags = Dumplog::PredefinedHeaderFlags;
    return static_cast<Flags>(static_cast<std::underlying_type_t<Flags>>(lhs) |
                              static_cast<std::underlying_type_t<Flags>>(rhs));
}

}  // namespace xoj::util::log
