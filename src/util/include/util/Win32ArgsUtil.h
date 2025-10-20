/*
 * Xournal++
 *
 * Windows UTF-8 Arguments Helper
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#ifdef _WIN32

/**
 * Converts Windows wide-character arguments to UTF-8
 * This is required for proper handling of Unicode filenames (e.g., ä, ö, ü, 中文)
 *
 * @param argc Reference to argc that will be updated with the converted argument count
 * @param argv Reference to argv that will be updated with the converted argument pointers
 * @return true if conversion was successful and argc/argv were updated
 */
bool convertWin32ArgsToUtf8(int& argc, char**& argv);

#endif
