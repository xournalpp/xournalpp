#include "Path.h"

#include "StringUtils.h"

#include <glib/gstdio.h>

#include <cstring>
#include <utility>

Path::Path(string path)
 : path(std::move(path))
{
}

Path::Path(const char* path)
 : path(path)
{
}

Path& Path::operator=(string p)
{
	this->path = std::move(p);
	return *this;
}

Path& Path::operator=(const char* p)
{
	this->path = p;
	return *this;
}

Path& Path::operator/=(const Path& p)
{
	return *this /= p.str();
}

Path& Path::operator/=(const string& p)
{
	if (!path.empty())
	{
		path.reserve(path.size() + p.size() + 1);
		path += G_DIR_SEPARATOR_S;
	}
	path += p;
	return *this;
}

Path& Path::operator/=(const char* p)
{
	if (!path.empty())
	{
		char c = path.at(path.size() - 1);
		if (c != '/' && c != '\\')
		{
			path += G_DIR_SEPARATOR_S;
		}
	}
	path += p;
	return *this;
}

Path& Path::operator+=(const Path& p)
{
	return *this += p.str();
}

Path& Path::operator+=(const string& p)
{
	path += p;
	return *this;
}

Path& Path::operator+=(const char* p)
{
	path += p;
	return *this;
}

Path Path::operator/(const Path& p) const
{
	return *this / p.str();
}

Path Path::operator/(const string& p) const
{
	Path ret(*this);
	ret /= p;
	return ret;
}

Path Path::operator/(const char* p) const
{
	Path ret(*this);
	ret /= p;
	return ret;
}

bool Path::operator==(const Path& other) const
{
	return this->path == other.path;
}

bool Path::isEmpty() const
{
	return path.empty();
}

bool Path::exists() const
{
	return g_file_test(path.c_str(), G_FILE_TEST_EXISTS);
}

bool Path::deleteFile()
{
	return g_unlink(c_str()) == 0;
}

bool Path::hasXournalFileExt() const
{
	return hasExtension(".xoj") || hasExtension(".xopp");
}

bool Path::hasExtension(const string& ext) const
{
	if (ext.length() >= path.length())
	{
		return false;
	}
	auto index = path.find_last_of('.', path.length() - ext.length() - (ext.at(0) == '.' ? 0 : 1));
	if (index == std::string::npos)
	{
		return false;
	}

	string pathExt = path.substr(path.length() - ext.length());
	return StringUtils::toLowerCase(pathExt) == StringUtils::toLowerCase(ext);
}

void Path::clearExtensions(const string& ext)
{
	string plower = StringUtils::toLowerCase(path);
	auto rm_ext = [&](string const& ext) {
		if (StringUtils::endsWith(plower, ext))
		{
			auto newLen = plower.length() - ext.size();
			if (newLen < path.length())
			{
				this->path = path.substr(0, newLen);
			}
		}
	};
	rm_ext(".xoj");
	rm_ext(".xopp");
	if (!ext.empty())
	{
		string extLower = StringUtils::toLowerCase(ext);
		rm_ext(extLower);
		rm_ext(extLower + ".xoj");
		rm_ext(extLower + ".xopp");
	}
}

const string& Path::str() const
{
	return path;
}

const char* Path::c_str() const
{
	return path.c_str();
}

string Path::getEscapedPath() const
{
	string escaped = path;
	StringUtils::replaceAllChars(escaped, {replace_pair('\\', "\\\\"), replace_pair('\"', "\\\"")});

	return escaped;
}

string Path::getFilename() const
{
	size_t separator = path.find_last_of("/\\");

	if (separator == string::npos)
	{
		return str();
	}

	return path.substr(separator + 1);
}

string Path::toUri(GError** error)
{
	char* uri = g_filename_to_uri(path.c_str(), nullptr, error);

	if (uri == nullptr)
	{
		return {};
	}

	string uriString = uri;
	g_free(uri);
	return uriString;
}

#ifndef BUILD_THUMBNAILER

GFile* Path::toGFile()
{
	return g_file_new_for_path(path.c_str());
}
#endif

Path Path::getParentPath() const
{
	size_t separator = path.find_last_of("/\\");

	if (separator == string::npos)
	{
		return {};
	}

	return Path{path.substr(0, separator)};
}

Path Path::fromUri(const string& uri)
{
	if (!StringUtils::startsWith(uri, "file://"))
	{
		return {};
	}

	gchar* filename = g_filename_from_uri(uri.c_str(), nullptr, nullptr);
	Path p(filename);
	g_free(filename);

	return p;
}

#ifndef BUILD_THUMBNAILER
Path Path::fromGFile(GFile* file)
{
	char* uri = g_file_get_uri(file);
	Path ret{fromUri(uri)};
	g_free(uri);

	return ret;
}
#endif
