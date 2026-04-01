#pragma once

#include <locale>
#include <utility>

/**
 * @brief Serdes always requires the same locale. Independently of the local / C++ global locale.
 */

template <typename StdStream, typename... Args>
StdStream serdes_stream(Args&&... args) {
    StdStream ret(std::forward<Args>(args)...);
    ret.imbue(std::locale::classic());
    return ret;
}