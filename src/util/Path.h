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

#include <glib.h>
#include <gio/gio.h>

#include <string>
using std::string;

class Path
{
public:
	Path();
	Path(const Path& other);
	Path(string path);
	Path(const char* path);
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
	 * Delete the file
	 *
	 * @return true if the file is deleted or does not exists
	 */
	bool deleteFile();

	/**
	 * Compare the path with another one
	 */
	bool operator ==(const Path& other);

	/**
	 * Assign path
	 */
	void operator =(const Path& other);

	/**
	 * Assign path
	 */
	void operator =(const string& path);

	/**
	 * Assign path
	 */
	void operator =(const char* path);

	/**
	 * Check if the path ends with this extension
	 *
	 * @param ext Extension, needs to be lowercase
	 * @return true if the extension is there
	 */
	bool hasExtension(string ext);

	/**
	 * Clear the extension (last .xyz or .pdf.xoj, .pdf.xopp)
	 */
	void clearExtensions();

	/**
	 * @return true if this file has .xopp or .xoj extension
	 */
	bool hasXournalFileExt();

	/**
	 * Return the Path as String
	 */
	const string str() const;

	/**
	 * Return the Path as String
	 */
	const char* c_str() const;

	/**
	 * Get the parent path
	 */
	Path getParentPath();

	/**
	 * Return the Filename of the path
	 */
	string getFilename();

	/**
	 * Convert this path to Uri
	 */
	string toUri(GError** error = NULL);

	/**
	 * Get escaped path, all " and \ are escaped
	 */
	string getEscapedPath();

	// Append operations
public:
	void operator /=(Path p);
	void operator /=(string p);
	void operator /=(const char* p);

	void operator +=(Path p);
	void operator +=(string p);
	void operator +=(const char* p);


public:
	/**
	 * Convert an uri to a path, if the uri does not start with file:// an empty Path is returned
	 */
	static Path fromUri(string uri);
	static Path fromGFile(GFile* file);

private:
	string path;
};
