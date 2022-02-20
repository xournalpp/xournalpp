// FIXME Put under BSL-1.0?

#pragma once

#ifdef _WIN32

#include <cassert>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>

#include "util/safe_casts.h"

#include "filesystem.h"


// Declarations from windows.h
extern "C" {

typedef void* HANDLE;
typedef HANDLE HLOCAL;

typedef struct HINSTANCE__* HINSTANCE;
typedef HINSTANCE HMODULE;

__declspec(dllimport) int __stdcall CloseHandle(HANDLE);
__declspec(dllimport) HLOCAL __stdcall LocalFree(HLOCAL);

typedef struct _CONTEXT CONTEXT;

}  // extern "C"


namespace xoj::win32 {

////////////////////////////////////////////////////////////////////////////////
// Win32 std::system_error
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Strong type for Win32 error codes
 */
struct win32_errc {
    constexpr explicit win32_errc(unsigned long ec): ec_(ec) {}

    [[nodiscard]] constexpr auto value() const -> unsigned long { return ec_; }
    [[nodiscard]] constexpr auto intValue() const -> int {
        static_assert(sizeof(ec_) == sizeof(int));
        return static_cast<int>(ec_);
    }

    /**
     * @brief Convenience function to directly get the corresponding system error message.
     *
     * Equivalent to all of the following:
     *     ::win32_category().message(this->intValue());
     *     std::error_code(*this).message();
     */
    [[nodiscard]] auto message() const -> std::string;

private:
    unsigned long ec_;
};

[[maybe_unused]] [[nodiscard]] auto win32_category() noexcept -> const std::error_category&;

[[maybe_unused]] [[nodiscard]] auto make_error_code(win32_errc ec) noexcept -> std::error_code;

}  // namespace xoj::win32

namespace std {

// enable implicit conversion from xoj::win32::win32_errc to std::error_code
template <>
struct is_error_code_enum<xoj::win32::win32_errc>: std::true_type {};

}  // namespace std

namespace xoj::win32 {

/**
 * @return the calling thread's last error code value
 */
[[maybe_unused]] [[nodiscard]] auto getLastError() -> win32_errc;

/**
 * @brief std::system_error extension for Win32 errors.
 *
 * In addition to `whatArg` specified in constructor the message returned by the `what()` member function will contain
 * the system error message corresponding to the error code.
 */
class win32_error: public std::system_error {
public:
    win32_error(win32_errc ec, const char* whatArg);

    /**
     * @brief Construct win32_error from raw error code
     */
    win32_error(unsigned long ec, const char* whatArg);

    /**
     * @brief Constructs win32_error using the calling thread's last error code
     *
     * Equivalent to
     *
     *     win32_error(getLastError(), ...)
     */
    explicit win32_error(const char* whatArg);
};

////////////////////////////////////////////////////////////////////////////////
// Win32 style stacktrace
////////////////////////////////////////////////////////////////////////////////

[[maybe_unused]] __declspec(noinline) void printStacktrace(std::ostream& stream, HANDLE process, HANDLE thread,
                                                           unsigned short skipFrames, unsigned short maxFrames);
[[maybe_unused]] __declspec(noinline) void printStacktrace(std::ostream& stream, unsigned short skipFrames,
                                                           unsigned short maxFrames);
[[maybe_unused]] void printStacktrace(std::ostream& stream, HANDLE process, HANDLE thread, unsigned short maxFrames,
                                      CONTEXT* context);

////////////////////////////////////////////////////////////////////////////////
// Resource handling
////////////////////////////////////////////////////////////////////////////////

enum class DeleterFunctionType : bool { zeroIsSuccess = false, nonzeroIsSuccess = true };

namespace detail {

template <class T, auto DeleterF, DeleterFunctionType DeleterFT>
class SystemResourceDeleter {
    using DeleterF_t = decltype(DeleterF);

    static_assert(std::is_invocable_r_v<bool, DeleterF_t, T*>);

public:
    void operator()(T* resource) const {
        if (static_cast<bool>(DeleterF(resource)) != static_cast<bool>(DeleterFT)) {
            // Will terminate the process since unique_ptr::reset is noexcept, but the uncaught
            // exception will at least print a helpful message with libstdc++ and libc++.
            throw win32_error("Deleter function failed");
        }
    }
};

}  // namespace detail

template <class T, auto DeleterF, DeleterFunctionType DeleterFT>
class UniqueResource: public std::unique_ptr<T, detail::SystemResourceDeleter<T, DeleterF, DeleterFT>> {
    using unique_ptr = std::unique_ptr<T, detail::SystemResourceDeleter<T, DeleterF, DeleterFT>>;

public:
    UniqueResource() = default;
    UniqueResource(const UniqueResource&) = delete;
    UniqueResource(UniqueResource&&) = default;
    UniqueResource& operator=(const UniqueResource&) = delete;
    UniqueResource& operator=(UniqueResource&&) = default;
    ~UniqueResource() = default;

    UniqueResource(T* resource): unique_ptr(resource) {}

    /**
     * @brief Proxy to be passed to function with `T**` or `T*&` output parameter. See `UniqueResource::oref()`.
     */
    struct OutputRef {
        // OutputRefs are not supposed to be passed around, rely on copy elision in intended use cases
        OutputRef(const OutputRef&) = delete;
        OutputRef(OutputRef&&) = delete;
        OutputRef& operator=(const OutputRef&) = delete;
        OutputRef& operator=(OutputRef&&) = delete;
        ~OutputRef() {
            if (ptr_ != ur_.get())
                ur_.reset(ptr_);
        }

        [[nodiscard]] auto ptr() -> T** { return &ptr_; }
        operator T**() { return &ptr_; }

        [[nodiscard]] auto ref() -> T*& { return ptr_; }
        operator T*&() { return ptr_; }

    private:
        OutputRef(UniqueResource& ur): ur_(ur), ptr_(ur.get()) {}

        UniqueResource& ur_;
        T* ptr_;

        friend class UniqueResource;
    };

    /**
     * @brief Returns a proxy object that can be used with functions with a `T**` or `T*&` output parameter.
     *
     * The proxy object contains a reference to `*this` and a pointer that can be accessed via implicit conversion to
     * `T**` and `T*&` or explicitly using the `OutputRef::ptr()` and `OutputRef::ref()` member functions.
     *
     * If the pointer in the proxy object has changed, this UniqueResource is reset to the new pointer when the proxy
     * object is destroyed.
     *
     * Usage:
     *
     *     bool funcWithPtrOutputParam(void** outputParam);
     *     bool funcWithRefOutputParam(void*& outputParam);
     *
     *     UniqueResource<void, ...> res;
     *     bool r1 = funcWithPtrOutputParam(res.oref());
     *     // or explicitly: ... funcWithPtrOutputParam(res.oref().ptr());
     *
     *     bool r2 = funcWithRefOutputParam(res.oref());
     *     // or explicitly: ... funcWithRefOutputParam(res.oref().ref());
     *
     */
    [[nodiscard]] auto oref() -> OutputRef { return OutputRef(*this); }
};

using UniqueHandle = UniqueResource<void, &CloseHandle, DeleterFunctionType::nonzeroIsSuccess>;

template <class T>
using UniqueLocal = UniqueResource<T, &LocalFree, DeleterFunctionType::zeroIsSuccess>;

////////////////////////////////////////////////////////////////////////////////
// Misc utils
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Returns the fully qualified path for the file that contains the specified module.
 *
 * The module must have been loaded by the current process.
 */
[[maybe_unused]] [[nodiscard]] auto getModuleFileName(HMODULE hModule) -> fs::path;

/**
 * @brief Strong type for infinite timeout
 */
struct infinite_t {};

/**
 * @brief Strongly typed constant for infinite timeout
 */
constexpr infinite_t infinite;

/**
 * @brief Variant type that can either hold a timeout (specified in milliseconds) or an infinite timeout
 */
using WaitTimeout = std::variant<unsigned long, infinite_t>;

class WaitResult;

namespace detail {

/**
 * @brief Implementation for `waitForSingleObject(...)` and `waitForMultipleObjects(...)`
 */
auto waitForObjects(unsigned long count, const HANDLE* objects, bool all, WaitTimeout timeout) -> WaitResult;

}  // namespace detail

/**
 * @brief Result object of a wait operation.
 *
 * See `waitForSingleObject(...)` and `waitForMultipleObjects(...)`
 */
class WaitResult {
public:
    WaitResult(const WaitResult&) = default;
    WaitResult(WaitResult&&) = default;
    WaitResult& operator=(const WaitResult&) = default;
    WaitResult& operator=(WaitResult&&) = default;
    ~WaitResult() = default;

    bool done() const;
    bool isAbandoned() const;
    unsigned long index() const;

    explicit operator bool() const { return done(); }

private:
    WaitResult() = default;

    unsigned long index_ = 0;
    unsigned char flags_ = 0;

    friend auto detail::waitForObjects(unsigned long, const HANDLE*, bool, WaitTimeout) -> WaitResult;
};

/**
 * @brief Blocks until the object is signaled or the timeout is exceeded.
 *
 * The Result object has the following properties:
 * * The `done()` member function (or implicit converion to `bool`) returns `true` if the object is signaled.
 *   - Then the `isAbandoned()` member function will return `true` iff the object is a mutex that was owned by a process
 *     that terminated before properly releasing the mutex.
 * * The `done()` member function (or implicit converion to `bool`) returns `false` if the wait operation returned
 *   because the timeout was exceeded.
 *
 * If the wait operation fails, a `::win32_error` is thrown.
 *
 * @param object Object to wait for.
 * @param timeout Timeout (in milliseconds) or `::infinite`
 */
[[maybe_unused]] [[nodiscard]] inline auto waitForSingleObject(HANDLE object, WaitTimeout timeout) -> WaitResult {
    return detail::waitForObjects(1, &object, true, timeout);
}

/**
 * @brief Blocks until one object / all objects are signaled or the timeout is exceeded.
 *
 * The Result object has the following properties:
 * * The `done()` member function (or implicit converion to `bool`) returns `true` if - depending on the `all`
 *   parameter - one or all objects are signaled.
 *   - Then the `isAbandoned()` member function will return true iff at least one object is a Win32 mutex that was owned
 *     by a process that terminated before properly releasing the mutex.
 *   If the `all` parameter is set to `false`, the `index()` member function will return the index of the object within
 *   `objects` that has been signaled. Otherwise it will return some object index.
 * * The `done()` member function (or implicit converion to `bool`) returns `false` if the wait operation returned
 *   because the timeout was exceeded.
 *
 * If the wait operation fails, a `::win32_error` is thrown.
 *
 * @tparam T container type that satisfies
 *     { data(objects) } -> convertible_to<HANDLE*>;
 *     { size(objects) } -> convertible_to<size_t>;
 * @param objects Objects to wait for. `size(objects)` must be within the limits of `unsigned long`.
 * @param all If `true`, block until all objects are signaled, else return when one object is signaled.
 * @param timeout Timeout (in milliseconds) or `::infinite`
 */
template <class T>
[[maybe_unused]] [[nodiscard]] auto waitForMultipleObjects(const T& objects, bool all, WaitTimeout timeout)
        -> WaitResult {
    static_assert(std::is_convertible_v<decltype(data(objects)), const HANDLE*>);
    static_assert(std::is_convertible_v<decltype(size(objects)), size_t>);

    const auto count = size(objects);
    assert(count > 0);
    return detail::waitForObjects(strict_cast<unsigned long>(count), data(objects), all, timeout);
}
[[maybe_unused]] [[nodiscard]] inline auto waitForMultipleObjects(std::initializer_list<HANDLE>&& objects, bool all, WaitTimeout timeout) {
    return waitForMultipleObjects(objects, all, timeout);
}

}  // namespace xoj::win32

#endif  // _WIN32
