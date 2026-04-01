#include "setup-env.h"

#include <string>

#include <CoreFoundation/CoreFoundation.h>
#include <glib.h>
#include <poppler-global.h>

#include "util/PathUtil.h"

#include "filesystem.h"

void setupEnvironment() {
    /**
     * see https://gitlab.gnome.org/GNOME/gtk-mac-bundler/-/blob/master/examples/gtk3-launcher.sh
     * and https://gitlab.gnome.org/GNOME/gtk-mac-bundler/-/issues/12
     */
    auto base = Util::getExePath().parent_path();  // Xournal++.app/Contents or $HOME/gtk/inst

    if (fs::exists(base / "Resources")) {  // app-bundle
        base = base / "Resources";
        // Set poppler data directory for (relocatable) app bundles. Otherwise a compile time value is used.
        auto popplerDataDir = base / "share" / "poppler";
        poppler::set_data_dir(popplerDataDir.string().c_str());
    }  // Now base is Xournal++.app/Contents/Resources or $HOME/gtk/inst

    auto libPath = base / "lib";
    auto dataPath = base / "share";
    auto xdgPath = base / "etc" / "xdg";

    auto imModuleFile = libPath / "gtk-3.0" / "3.0.0" / "immodules.cache";
    auto pixbufModuleFile = libPath / "gdk-pixbuf-2.0" / "2.10.0" / "loaders.cache";

    auto typelibPath = libPath / "girepository-1.0";

    std::string luaPath = dataPath.string() + "/lua/5.4/?.lua";
    std::string luaCPath = libPath.string() + "/lua/5.4/?.so";

    setenv("XDG_CONFIG_DIRS", xdgPath.string().c_str(), 1);
    setenv("XDG_DATA_DIRS", dataPath.string().c_str(), 1);
    setenv("GTK_DATA_PREFIX", base.string().c_str(), 1);
    setenv("GTK_EXE_PREFIX", base.string().c_str(), 1);
    setenv("GTK_PATH", base.string().c_str(), 1);

    setenv("GTK_IM_MODULE_FILE", imModuleFile.string().c_str(), 0);
    setenv("GDK_PIXBUF_MODULE_FILE", pixbufModuleFile.string().c_str(), 0);

    setenv("GI_TYPELIB_PATH", typelibPath.string().c_str(), 0);

    setenv("LUA_PATH", luaPath.c_str(), 0);
    setenv("LUA_CPATH", luaCPath.c_str(), 0);

    auto environ = g_get_environ();
    const char* usedPixbufModuleFile = g_environ_getenv(environ, "GDK_PIXBUF_MODULE_FILE");
    const char* usedTypelibPath = g_environ_getenv(environ, "GI_TYPELIB_PATH");
    // The DYLD_LIBRARY_PATH is only read when the process is started, so it can't be set here. It is set in
    // the Info.plist therefore, which only takes effect when running the App from Finder or using the "open" command.
    const char* usedDYLDLibraryPath = g_environ_getenv(environ, "DYLD_LIBRARY_PATH");
    const char* usedLuaPath = g_environ_getenv(environ, "LUA_PATH");
    const char* usedLuaCPath = g_environ_getenv(environ, "LUA_CPATH");
    g_message("Continue with GDK_PIXBUF_MODULE_FILE = %s\n"
              "GI_TYPELIB_PATH = %s\n"
              "DYLD_LIBRARY_PATH = %s\n"
              "LUA_PATH = %s\n"
              "LUA_CPATH = %s",
              usedPixbufModuleFile, usedTypelibPath, usedDYLDLibraryPath, usedLuaPath, usedLuaCPath);

    /**
     * set LANG and LC_MESSAGES in order to detect the default language
     * Use the equivalent of 'defaults read -g "AppleLanguages"' executed in the terminal, also visible in System
     * settings -> Language and Region
     */
    CFArrayRef languages =
            (CFArrayRef)CFPreferencesCopyAppValue(CFSTR("AppleLanguages"), kCFPreferencesCurrentApplication);
    if (languages && CFArrayGetCount(languages)) {
        CFStringRef langCode = (CFStringRef)CFArrayGetValueAtIndex(languages, 0);
        if (langCode) {
            char buffer[128];
            if (CFStringGetCString(langCode, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
                std::string lang = buffer;  // e.g. "de-DE"
                if (auto pos = lang.find("-"); pos != std::string::npos) {
                    lang.replace(pos, 1, "_");  // e.g. "de_DE"
                }
                lang += ".UTF-8";  // e.g. "de_DE.UTF-8"
                g_message("Setting LANG and LC_MESSAGES to %s", lang.c_str());
                setenv("LANG", lang.c_str(), 0);
                setenv("LC_MESSAGES", lang.c_str(), 0);
            }
        }
    }
}
