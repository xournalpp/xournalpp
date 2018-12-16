/*
 * Xournal++
 *
 * Launcher to start Xournal++ in the correct dir on Windows
 * Without this, pressure will not work
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <stdio.h>
#include <stdlib.h>

#include <string>
using std::string;

#ifdef WIN32
#include <Windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

string escapeString(const char* str)
{
	string escaped;

	while (*str)
	{
		char c = *str;

		if (c == '"')
		{
			escaped += "\\\"";
		}
		else
		{
			escaped += c;
		}

		str++;
	}

	return escaped;
}

int main(int argc, char* argv[])
{
	string exePath;

#ifdef WIN32
	char szFileName[MAX_PATH + 1];
	GetModuleFileNameA(NULL, szFileName, MAX_PATH + 1);
	exePath = szFileName;
#else
	char result[1024];
	ssize_t count = readlink("/proc/self/exe", result, 1024);
	exePath = string(result, (count > 0) ? count : 0);
#endif

	int slashPos = 0;

	for(int i = exePath.size(); i > 0; i--)
	{
		if (exePath[i] == '/' || exePath[i] == '\\')
		{
			slashPos = i;
			break;
		}
	}

	string folder = exePath.substr(0, slashPos);

	chdir(folder.c_str());

	string command = "xournalpp_bin.exe";

	for (int i = 1; i < argc; i++)
	{
		command += " \"";
		command += escapeString(argv[i]);
		command += "\"";
	}

	system(command.c_str());

	return 0;
}


