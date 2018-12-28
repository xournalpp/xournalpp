#include "Path.h"
#include "StringUtils.h"

Path::Path()
{
	XOJ_INIT_TYPE(Path);
}

Path::Path(const Path& other)
 : path(other.path)
{
	XOJ_INIT_TYPE(Path);
}

Path::Path(string path)
 : path(path)
{
	XOJ_INIT_TYPE(Path);
}

Path::~Path()
{
	XOJ_CHECK_TYPE(Path);
	XOJ_RELEASE_TYPE(Path);
}

/**
 * @return true if empty
 */
bool Path::isEmpty()
{
	XOJ_CHECK_TYPE(Path);

	return path.empty();
}

/**
 * Check if this is a file which exists
 */
bool Path::exists()
{
	XOJ_CHECK_TYPE(Path);

	return g_file_test(path.c_str(), G_FILE_TEST_EXISTS);
}

/**
 * Check if the path ends with this extension
 */
bool Path::hasExtension(string ext)
{
	XOJ_CHECK_TYPE(Path);

	return StringUtils::endsWith(path, ext);
}

/**
 * Return the Path as String
 */
string Path::str()
{
	XOJ_CHECK_TYPE(Path);

	return path;
}

/**
 * Return the Path as String
 */
const char* Path::c_str()
{
	XOJ_CHECK_TYPE(Path);

	return path.c_str();
}

/**
 * Get the parent path
 */
Path Path::getParentPath()
{
	XOJ_CHECK_TYPE(Path);

	size_t separator = path.find_last_of("/\\");

	if (separator == string::npos)
	{
		return *this;
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



