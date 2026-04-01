/*
 * Xournal++
 *
 * Helper functions for libsndfile
 * platform dependent code to open std::filesystem::paths
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#ifdef _WIN32
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif

#include <memory>

#include <sndfile.h>

#include "filesystem.h"

namespace xoj::audio {

struct SNDFILE_Deleter {
    void operator()(SNDFILE* file) { sf_close(file); }
};

using SNDFileGuard = std::unique_ptr<SNDFILE, SNDFILE_Deleter>;


/**
 * Open a file with libsndfile
 *
 * @param path std::string const& path to ensure system single byte encoding
 * @return The resource pointer to the sound file
 */
inline SNDFILE* sf_open_pp(std::string const& path, int mode, SF_INFO* info) {
    return sf_open(path.c_str(), mode, info);
}
#if (defined(ENABLE_SNDFILE_WINDOWS_PROTOTYPES) && ENABLE_SNDFILE_WINDOWS_PROTOTYPES)
/**
 * Open a file with libsndfile
 *
 * @param path std::wstring const& path to ensure system double byte encoding
 * @return The resource pointer to the sound file
 */
inline SNDFILE* sf_open_pp(std::wstring const& path, int mode, SF_INFO* info) {
    return sf_wchar_open(path.c_str(), mode, info);
}
#endif

// naming similar to std::make_unique
inline auto make_snd_file(fs::path const& file, int mode, SF_INFO* info) {
    return SNDFileGuard{sf_open_pp(file, mode, info)};
}

}  // namespace xoj::audio