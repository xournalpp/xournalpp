#include "GladeSearchpath.h"

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

char* GladeSearchpath::findFile(const char* subdir, const char* file)
{
	XOJ_CHECK_TYPE(GladeSearchpath);

	char* filename = NULL;
	if (subdir == NULL)
	{
		filename = g_strdup(file);
	}
	else
	{
		filename = g_strdup_printf("%s%c%s", subdir, G_DIR_SEPARATOR, file);
	}

	// We step through each directory to find it.
	for (string str : directories)
	{
		gchar* pathname = g_strdup_printf("%s%c%s", str.c_str(), G_DIR_SEPARATOR, filename);

		if (g_file_test(pathname, G_FILE_TEST_EXISTS))
		{
			g_free(filename);
			return pathname;
		}

		g_free(pathname);
	}

	g_free(filename);
	return NULL;
}

/*
 * Use this function to set the directory containing installed pixmaps and Glade XML files.
 */
void GladeSearchpath::addSearchDirectory(string directory)
{
	XOJ_CHECK_TYPE(GladeSearchpath);

	this->directories.push_back(directory);
}
