#include "PathUtil.h"
#include "XojMsgBox.h"

/**
 * Read a file to a string
 *
 * @param output Read contents
 * @param path Path to read
 * @param showErrorToUser Show an error to the user, if the file could not be read
 *
 * @return true if the file was read, false if not
 */
bool PathUtil::readString(string& output, Path& path, bool showErrorToUser)
{
	gchar* contents = NULL;
	gsize length = 0;
	GError* error = NULL;
	if (g_file_get_contents(path.c_str(), &contents, &length, &error))
	{
		output = contents;
		g_free(contents);
		return true;
	}
	else
	{
		if (showErrorToUser)
		{
			XojMsgBox::showErrorToUser(NULL, error->message);
		}

		g_error_free(error);
		return false;
	}
}


