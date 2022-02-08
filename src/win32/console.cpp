#include "console.h"

#include <cstdio>

#include <windows.h>
//
// #include <wchar.h>
// #include <wincon.h>


void attachConsole() {
    if (GetConsoleWindow() != NULL) {
        // Console is already attached.
        return;
    }
    // https://www.tillett.info/2013/05/13/how-to-create-a-windows-program-that-works-as-both-as-a-gui-and-console-application/
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE &&
            GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE) {
            // Redirect stdout and stderr to the console.
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            return;
        }
    }

    // Connection failed, create a new console.

    // Make sure the console starts hidden.
    STARTUPINFOW startupInfo = {0};
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE;
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInformation = {0};

    bool consoleAttached = false;

    if (CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo,
                       &processInformation)) {
        HANDLE job = CreateJobObject(NULL, NULL);

        if (job != NULL) {
            // Terminate the console process automatically when Xournal++ exits.
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInformation = {0};
            jobInformation.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

            if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jobInformation,
                                         sizeof(jobInformation)) ||
                !AssignProcessToJobObject(job, processInformation.hProcess)) {
                TerminateProcess(processInformation.hProcess, 0);
                CloseHandle(processInformation.hProcess);
                CloseHandle(processInformation.hThread);
                CloseHandle(job);
                processInformation.hProcess = processInformation.hThread = job = NULL;
            } else {
                // It takes a short time before the console becomes ready to be attached, unfortunately
                // there is no API that would let us wait for it.
                for (unsigned short i = 0; i < 20; i++) {
                    if (AttachConsole(processInformation.dwProcessId)) {
                        consoleAttached = true;
                        break;
                    }
                    Sleep(50);
                }
            }
        } else {
            TerminateProcess(processInformation.hProcess, 0);
            CloseHandle(processInformation.hProcess);
            CloseHandle(processInformation.hThread);
            processInformation.hProcess = processInformation.hThread = NULL;
        }

        if (processInformation.hProcess != NULL) {
            if (!consoleAttached) {
                TerminateProcess(processInformation.hProcess, 0);
                CloseHandle(job);
            }

            CloseHandle(processInformation.hProcess);
            CloseHandle(processInformation.hThread);
        }
    }

    if (!consoleAttached) {
        // Could not attach to the manually created console process, request one from the system instead.
        // This will make the console window flash briefly before we hide it.
        AllocConsole();
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }
}
