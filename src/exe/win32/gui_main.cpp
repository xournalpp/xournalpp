// Only for Windows GUI applications, compiled with MSVC.
#ifdef _MSC_VER

#include <cstdlib>

#include <combaseapi.h>
#include <windows.h>

auto main(int argc, char* argv[]) -> int;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    // Call main:
    return main(__argc, __argv);
}

#endif
