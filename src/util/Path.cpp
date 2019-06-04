#include "Path.h"
#include "StringUtils.h"

#include <glib/gstdio.h>
#include <string.h>

Path::Path()
{
}

Path::Path(const Path& other)
 : path(other.path)
{
}

Path::Path(string path)
 : path(path)
{
}

Path::Path(const char* path)
 : path(path)
{
}

Path::~Path()
{
}

/**
 * @return true if empty
 */
bool Path::isEmpty()
{
	return path.empty();
}

/**
 * Check if this is a file which exists
 */
bool Path::exists()
{
	return g_file_test(path.c_str(), G_FILE_TEST_EXISTS);
}

/**
 * Delete the file
 *
 * @return true if the file is deleted or does not exists
 */
bool Path::deleteFile()
{
	if (!exists())
	{
		// Does not exists, therefore cannot be deleted
		return true;
	}

	int result = g_unlink(c_str());
	if (result == 0)
	{
		return true;
	}

	return false;
}

/**
 * Compare the path with another one
 */
bool Path::operator ==(const Path& other)
{
	return this->path == other.path;
}

/**
 * Assign path
 */
void Path::operator =(const Path& other)
{
	this->path = other.path;
}

/**
 * Assign path
 */
void Path::operator =(const string& path)
{
	this->path = path;
}

/**
 * Assign path
 */
void Path::operator =(const char* path)
{
	this->path = path;
}

/**
 * @return true if this file has .xopp or .xoj extension
 */
bool Path::hasXournalFileExt()
{
	return hasExtension(".xoj") || hasExtension(".xopp");
}

/**
 * Check if the path ends with this extension
 *
 * @param ext Extension, needs to be lowercase
 * @return true if the extension is there
 */
bool Path::hasExtension(string ext)
{
	if (ext.length() > path.length())
	{
		return false;
	}

	string pathExt = path.substr(path.length() - ext.length());
	pathExt = StringUtils::toLowerCase(pathExt);

	return pathExt == ext;
}

#define REMOVE_EXTENSION(ext) \
	if (StringUtils::endsWith(plower, ext)) \
	{ \
		path = path.substr(0, path.length() - strlen(ext)); \
		return;\
	}

/**
 * Clear the the last known extension (last .pdf, .pdf.xoj, .pdf.xopp etc.)
 */
void Path::clearExtensions()
{
	string plower = StringUtils::toLowerCase(path);
	
	REMOVE_EXTENSION(".xoj");
	REMOVE_EXTENSION(".xopp");
}

/**
 * Return the Path as String
 */
const string Path::str() const
{
	return path;
}

/**
 * Return the Path as String
 */
const char* Path::c_str() const
{
	return path.c_str();
}

/**
 * Get escaped path, all " and \ are escaped
 */
string Path::getEscapedPath()
{
	string escaped = path;
	StringUtils::replaceAllChars(escaped, {
		replace_pair('\\', "\\\\"),
		replace_pair('\"', "\\\"")
	});

	return escaped;
}

void Path::operator /=(Path p)
{
	*this /= p.str();
}

void Path::operator /=(string p)
{
	if (path.size() > 0)
	{
		char c = path.at(path.size() - 1);
		if (c != '/' && c != '\\')
		{
			path += G_DIR_SEPARATOR_S;
		}
	}
	path += p;
}

void Path::operator /=(const char* p)
{
	*this /= string(p);
}

Path Path::operator /(Path p)
{
	return *this / p.c_str();
}

Path Path::operator /(string p)
{
	return *this / p.c_str();
}

Path Path::operator /(const char* p)
{
	Path ret(*this);
	ret /= p;
	return ret;
}

void Path::operator +=(Path p)
{
	path += p.str();
}

void Path::operator +=(string p)
{
	path += p;
}

void Path::operator +=(const char* p)
{
	path += p;
}

/**
 * Return the Filename of the path
 */
string Path::getFilename()
{
	size_t separator = path.find_last_of("/\\");

	if (separator == string::npos)
	{
		return str();
	}

	return path.substr(separator + 1);
}

/**
 * Convert this path to Uri
 */
string Path::toUri(GError** error)
{
	char * uri = g_filename_to_uri(path.c_str(), NULL, error);

	if (uri == NULL)
	{
		return "";
	}

	string uriString = uri;
	g_free(uri);
	return uriString;
}

#ifndef BUILD_THUMBNAILER
/**
 * Convert this path to GFile
 */
GFile* Path::toGFile()
{
	return g_file_new_for_path(path.c_str());
}
#endif

/**
 * Get the parent path
 */
Path Path::getParentPath()
{
	size_t separator = path.find_last_of("/\\");

	if (separator == string::npos)
	{
		return "";
	}

	return path.substr(0, separator);
}

/**
 * Convert an uri to a path, if the uri does not start with file:// an empty Path is returned
 */
Path Path::fromUri(string uri)
{
	if (!StringUtils::startsWith(uri, "file://"))
	{
		return Path();
	}

	gchar* filename = g_filename_from_uri (uri.c_str(), NULL, NULL);
	Path p(filename);
	g_free(filename);

	return p;
}

#ifndef BUILD_THUMBNAILER
Path Path::fromGFile(GFile* file)
{
	char* uri = g_file_get_uri(file);
	string sUri = uri;
	g_free(uri);

	return fromUri(sUri);
}
#endif
