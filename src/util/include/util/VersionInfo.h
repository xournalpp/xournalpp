/*
 * Xournal++
 *
 * get version info on various components and libraries
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <string>

namespace xoj::util {
/// Get the running GDK backend (or nullptr if none)
const char* getGdkBackend();

/// Get a string "Xournal++ a.b.c + commit info"
std::string getXournalppVersion();

/// Get a string describing the OS
std::string getOsInfo();

/// Get a paragraph with all version info
std::string getVersionInfo();
};  // namespace xoj::util
