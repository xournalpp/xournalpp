/*
 * Xournal++
 *
 * Path for file / folder handling
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XournalType.h"

class Path
{
public:
	Path();
	Path(const Path& other);
	Path(string path);
	virtual ~Path();

public:
	/**
	 * @return true if empty
	 */
	bool isEmpty();

	/**
	 * Check if this file / folder exists
	 */
	bool exists();

	/**
	 * Check if the path ends with this extension
	 */
	bool hasExtension(string ext);

	/**
	 * Return the Path as String
	 */
	string str();

	/**
	 * Return the Path as String
	 */
	const char* c_str();

	/**
	 * Get the parent path
	 */
	Path getParentPath();


public:
	/**
	 * Convert an uri to a path, if the uri does not start with file:// an empty Path is returned
	 */
	static Path fromUri(string uri);

private:
	XOJ_TYPE_ATTRIB;

	string path;
};
