/*
 * Xournal++
 *
 * Helper for reading / writing files
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Path.h"

class PathUtil
{
	// No instance allowed
private:
	PathUtil();

public:
	/**
	 * Read a file to a string
	 *
	 * @param output Read contents
	 * @param path Path to read
	 * @param showErrorToUser Show an error to the user, if the file could not be read
	 *
	 * @return true if the file was read, false if not
	 */
	static bool readString(string& output, Path& path, bool showErrorToUser = true);

	static bool copy(Path src, Path dest);
};
