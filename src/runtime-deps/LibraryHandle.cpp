#include "LibraryHandle.h"

#include "util/PathUtil.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <string>

using namespace xoj::runtime;

static constexpr const char* LIBRARY_PREFIX = "lib";

static LibraryHandle::Handle openLib(const fs::path& lib);
static std::string getLastError();
static bool closeLib(LibraryHandle::Handle handle);  ///< returns true on success
static void* loadSymbol(LibraryHandle::Handle handle, const char* symbolName);

#ifdef _WIN32
static constexpr const char* EXTENSION = ".dll";
static std::string getLastError() { return std::to_string(GetLastError()); }
static LibraryHandle::Handle openLib(const fs::path& lib) { return LoadLibraryW(lib.wstring().c_str()); }
static bool closeLib(LibraryHandle::Handle handle) { return FreeLibrary(handle); }
static void* loadSymbol(LibraryHandle::Handle handle, const char* symbolName) {
    return (void*)GetProcAddress(handle, symbolName);
}
#else
#ifdef __APPLE__
static constexpr const char* EXTENSION = ".dylib";
#else
static constexpr const char* EXTENSION = ".so";
#endif
static std::string getLastError() { return dlerror(); }
static LibraryHandle::Handle openLib(const fs::path& lib) { return dlopen(lib.string().c_str(), RTLD_LAZY); }
static void* loadSymbol(LibraryHandle::Handle handle, const char* symbolName) { return dlsym(handle, symbolName); }
static bool closeLib(LibraryHandle::Handle handle) { return dlclose(handle) == 0; }
#endif


LibraryHandle::LibraryHandle(const char* libCoreName): libCoreName(libCoreName) {
    auto libraryName(std::string(LIBRARY_PREFIX) + libCoreName + EXTENSION);
    fs::path libpath = Util::getOptionalComponentsPath() / fs::path(libraryName);

    this->handle = openLib(libpath);
    if (!this->handle) {
        g_warning("Failed to load \"%s\": %s", char_cast(libpath.u8string().c_str()), getLastError().c_str());
    }
}

LibraryHandle::~LibraryHandle() {
    if (this->handle) {
        closeLib(this->handle);
    }
}

LibraryHandle& LibraryHandle::operator=(LibraryHandle&& o) {
    handle = std::exchange(o.handle, nullptr);
    return *this;
}

LibraryHandle::LibraryHandle(LibraryHandle&& o): handle(std::exchange(o.handle, nullptr)) {}

void* LibraryHandle::getSymbol(const char* symbolName) const {
    void* sym = loadSymbol(this->handle, symbolName);
    if (!sym) {
        g_warning("Failed to load symbol \"%s\" from %s: %s", symbolName, this->libCoreName, getLastError().c_str());
    }
    return sym;
}
