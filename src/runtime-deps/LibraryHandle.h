/*
 * Xournal++
 *
 * Load a runtime-optional library
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

namespace xoj::runtime {
class LibraryHandle final {
public:
    LibraryHandle(const char* libCoreName);
    ~LibraryHandle();

    LibraryHandle(const LibraryHandle&) = delete;
    LibraryHandle& operator=(const LibraryHandle&) = delete;
    LibraryHandle(LibraryHandle&&);
    LibraryHandle& operator=(LibraryHandle&&);

    inline explicit operator bool() const { return handle; }


#ifdef _WIN32
    using Handle = HMODULE;
#else
    using Handle = void*;
#endif

    template <typename SymbolType>
    SymbolType getSymbol(const char* symbolName) {
        return static_cast<SymbolType>(getSymbol(symbolName));
    }

private:
    void* getSymbol(const char* symbolName) const;

    Handle handle;
    const char* libCoreName;
};
}  // namespace xoj::runtime
