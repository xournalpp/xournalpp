#include "GladeSearchpath.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

namespace bf = boost::filesystem;

GladeSearchpath::GladeSearchpath()
{
	XOJ_INIT_TYPE(GladeSearchpath);
}

GladeSearchpath::~GladeSearchpath()
{
	XOJ_CHECK_TYPE(GladeSearchpath);

	directories.clear();

	XOJ_RELEASE_TYPE(GladeSearchpath);
}

string GladeSearchpath::findFile(string subdir, string file)
{
	XOJ_CHECK_TYPE(GladeSearchpath);

	string filename;
	if (subdir == "")
	{
		filename = file;
	}
	else
	{
		filename = subdir + G_DIR_SEPARATOR_S + file;
	}

	// We step through each directory to find it.
	for (string dir : directories)
	{
		string pathname = dir + G_DIR_SEPARATOR_S + filename;

		if (bf::exists(bf::path(pathname)))
		{
			return pathname;
		}
	}

	return "";
}

/*
 * Use this function to set the directory containing installed pixmaps and Glade XML files.
 */
void GladeSearchpath::addSearchDirectory(string directory)
{
	XOJ_CHECK_TYPE(GladeSearchpath);

	this->directories.push_back(directory);
}
