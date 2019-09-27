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

#ifdef _WIN32
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

#ifdef _WIN32
	char szFileName[MAX_PATH + 1];
	GetModuleFileNameA(nullptr, szFileName, MAX_PATH + 1);
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
	MessageBoxA(nullptr, argv[i], "Debug IN", 0);
	
		command += " \"";
		command += escapeString(argv[i]);
		command += "\"";
	}


#ifdef _WIN32
	STARTUPINFO info = {};
	PROCESS_INFORMATION processInfo;
	char* cmd = new char[command.size() + 1];
	strncpy(cmd, command.c_str(), command.size());
	cmd[command.size()] = 0;
//	MessageBoxA(nullptr, cmd, "Debug", 0);
	if (CreateProcessA(nullptr, cmd, nullptr, nullptr, true, 0, nullptr, folder.c_str(), &info, &processInfo))
	{
		WaitForSingleObject(processInfo.hProcess, INFINITE);
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
	
	delete cmd;
#else
	system(command.c_str());
#endif

	return 0;
}


