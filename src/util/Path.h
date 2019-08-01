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
	Path() = default;
	Path(const Path& other) = default;
	Path(Path&& other) = default;

	// Converting constructor it is intended not to use explicit there until c++17 (return {some_string}; will work).
	Path(string path);       // NOLINT
	Path(const char* path);  // NOLINT

	~Path() = default;


public:
	/**
	 * @return true if empty
	 */
	bool isEmpty() const;

	/**
	 * Check if this file / folder exists
	 */
	bool exists() const;

	/**
 	 * Delete the file
	 *
	 * @return true if the file existed and is deleted, false if an error occurred
	 */
	bool deleteFile();

	bool operator==(const Path& other) const;

	Path& operator=(const Path& other) = default;
	Path& operator=(Path&& other) = default;
	Path& operator=(string path);
	Path& operator=(const char* path);

	/**
	 * Check if the path ends with this extension
	 *
	 * @param ext ,the extension, it can be eather with or without '.'
	 * @return true if the extension is there
	 */
	bool hasExtension(const string& ext) const;

	/**
	 * Clear the the last known xournal extension (last .xoj, .xopp etc.)
	 *
	 * @param ext An extension to clear additionally, eg .pdf (would also clear
	 *  .pdf.xopp etc.)
	 */
	void clearExtensions(const string& ext = "");

	/**
	 * @return true if this file has .xopp or .xoj extension
	 */
	bool hasXournalFileExt() const;

	/**
	 * Return the Path as String
	 */
	const string& str() const;

	/**
	 * Return the Path as String
	 */
	const char* c_str() const;

	/**
	 * Get the parent path
	 */
	Path getParentPath() const;

	/**
	 * Return the Filename of the path
	 */
	string getFilename() const;

	/**
	 * Convert this path to Uri
	 */
	string toUri(GError** error = nullptr);

#ifndef BUILD_THUMBNAILER
	/**
	 * Convert this path to GFile
	 */
	GFile* toGFile();
#endif

	/**
	 * Get escaped path, all " and \ are escaped
	 */
	string getEscapedPath() const;

	// Append operations
public:
	/**
	 * Modifies this path by appending the other path.
	 */
	Path& operator/=(const Path& p);
	Path& operator/=(const string& p);
	Path& operator/=(const char* p);

	/**
	 * Creates a copy of this path with the other path appended.
	 *
	 * If this method is going to be used multiple times, instead use /= to
	 * avoid making multiple copies.
	 */
	Path operator/(const Path& p) const;
	Path operator/(const string& p) const;
	Path operator/(const char* p) const;

	Path& operator+=(const Path& p);
	Path& operator+=(const string& p);
	Path& operator+=(const char* p);

public:
	/**
	 * Convert an uri to a path, if the uri does not start with file:// an empty Path is returned
	 */
	static Path fromUri(const string& uri);
#ifndef BUILD_THUMBNAILER
	static Path fromGFile(GFile* file);
#endif

private:
	string path;
};
