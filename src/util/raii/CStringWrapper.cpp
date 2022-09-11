#include "util/raii/CStringWrapper.h"

#include <utility>

#include <glib.h>

namespace xoj::util {
inline namespace raii {
OwnedCString::OwnedCString(OwnedCString&& s): data(std::exchange(s.data, nullptr)) {}
OwnedCString& OwnedCString::operator=(OwnedCString&& s) {
    g_free(data);
    data = std::exchange(s.data, nullptr);
    return *this;
}

OwnedCString OwnedCString::assumeOwnership(char* s) {
    OwnedCString res;
    res.data = s;
    return res;
}
OwnedCString::~OwnedCString() { g_free(data); }

const char* OwnedCString::get() const { return data; }
OwnedCString::operator bool() const { return data; }

OwnedCString::operator std::string_view() const { return data ? data : std::string_view(); }

const char& OwnedCString::operator[](size_t n) const { return data[n]; }

char** OwnedCString::contentReplacer() {
    g_free(std::exchange(data, nullptr));
    return &data;
}
};  // namespace raii
};  // namespace xoj::util
