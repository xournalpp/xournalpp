#include "PathUtil.h"
#include "XojMsgBox.h"

#include <glib.h>
#include <glib/gstdio.h>

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
	gchar* contents = nullptr;
	gsize length = 0;
	GError* error = nullptr;
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
			XojMsgBox::showErrorToUser(nullptr, error->message);
		}

		g_error_free(error);
		return false;
	}
}

bool PathUtil::copy(Path src, Path dest)
{
	char buffer[16384]; // 16k

	FILE* fpRead = g_fopen(src.c_str(), "rb");
	if (!fpRead)
	{
		return false;
	}

	FILE* fpWrite = g_fopen(dest.c_str(), "wb");
	if (!fpWrite)
	{
		fclose(fpRead);
		return false;
	}

	while (!feof(fpRead))
	{
		size_t bytes = fread(buffer, 1, sizeof(buffer), fpRead);
		if (bytes)
		{
			fwrite(buffer, 1, bytes, fpWrite);
		}
	}

	fclose(fpRead);
	fclose(fpWrite);

	return true;
}

