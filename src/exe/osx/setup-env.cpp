#include "setup-env.h"

#include <string>

#include <CoreFoundation/CoreFoundation.h>
#include <glib.h>
#include <poppler-global.h>

#include "util/PathUtil.h"

#include "filesystem.h"

namespace {
void prependPathToEnvVar(const char* name, fs::path const& path) {
    const char* currentVal = g_getenv(name);
    std::string newVar;
    if (currentVal == nullptr || strlen(currentVal) == 0) {
        newVar = path.string();
    } else {
        newVar = path.string() + ":" + currentVal;
    }
    g_setenv(name, newVar.c_str(), 1);
}

void prependPathToLua(const char* name, const std::string& path) {
    const char* currentVal = g_getenv(name);
    std::string newVar;
    if (currentVal == nullptr || strlen(currentVal) == 0) {
        newVar = path + ";;";
    } else {
        newVar = path + ";" + currentVal;
    }
    g_setenv(name, newVar.c_str(), true);
}
}  // end namespace

void setupEnvironment() {
    /**
     * see https://gitlab.gnome.org/GNOME/gtk-mac-bundler/-/blob/master/examples/gtk3-launcher.sh
     * and https://gitlab.gnome.org/GNOME/gtk-mac-bundler/-/issues/12
     */
    auto base = Util::getExePath().parent_path();  // Xournal++.app/Contents or $HOME/gtk/inst or e.g. build/inst (via
                                                   // Homebrew or MacPorts)

    bool isAppBundle = fs::exists(base / "Resources");
    bool underJHBuild = g_strcmp0(g_getenv("UNDER_JHBUILD"), "true") == 0;

    if (isAppBundle) {
        base = base / "Resources";  // Now base is Xournal++.app/Contents/Resources
    }

    auto libPath = base / "lib";
    auto dataPath = base / "share";

    if (isAppBundle) {
        // Set poppler data directory for (relocatable) app bundles. Otherwise a compile time value is used.
        auto popplerDataDir = dataPath / "poppler";
        poppler::set_data_dir(popplerDataDir.string().c_str());

        auto imModuleFile = libPath / "gtk-3.0" / "3.0.0" / "immodules.cache";
        auto pixbufModuleFile = libPath / "gdk-pixbuf-2.0" / "2.10.0" / "loaders.cache";

        auto typelibPath = libPath / "girepository-1.0";
        auto xdgPath = base / "etc" / "xdg";

        prependPathToEnvVar("XDG_CONFIG_DIRS", xdgPath);
        prependPathToEnvVar("XDG_DATA_DIRS", dataPath);
        g_setenv("GTK_DATA_PREFIX", base.string().c_str(), 1);
        g_setenv("GTK_EXE_PREFIX", base.string().c_str(), 1);
        g_setenv("GTK_PATH", base.string().c_str(), 1);

        g_setenv("GTK_IM_MODULE_FILE", imModuleFile.string().c_str(), 0);
        g_setenv("GDK_PIXBUF_MODULE_FILE", pixbufModuleFile.string().c_str(), 0);

        g_setenv("GI_TYPELIB_PATH", typelibPath.string().c_str(), 0);
    }

    if (isAppBundle || underJHBuild) {  // in both cases we have non-standard paths for Lua
        std::string luaPath = dataPath.string() + "/lua/5.4/?.lua";
        std::string luaCPath = libPath.string() + "/lua/5.4/?.so";

        prependPathToLua("LUA_PATH", luaPath);
        prependPathToLua("LUA_CPATH", luaCPath);
    }

    const char* usedPixbufModuleFile = g_getenv("GDK_PIXBUF_MODULE_FILE");
    const char* usedXDGDataDirs = g_getenv("XDG_DATA_DIRS");
    const char* usedTypelibPath = g_getenv("GI_TYPELIB_PATH");
    // The DYLD_LIBRARY_PATH is only read when the process is started, so it can't be set here. It is set in
    // the Info.plist therefore, which only takes effect when running the App from Finder or using the "open" command.
    const char* usedDYLDLibraryPath = g_getenv("DYLD_LIBRARY_PATH");
    const char* usedLuaPath = g_getenv("LUA_PATH");
    const char* usedLuaCPath = g_getenv("LUA_CPATH");
    g_debug("Continue with:\n"
            "GDK_PIXBUF_MODULE_FILE = %s\n"
            "XDG_DATA_DIRS = %s\n"
            "GI_TYPELIB_PATH = %s\n"
            "DYLD_LIBRARY_PATH = %s\n"
            "LUA_PATH = %s\n"
            "LUA_CPATH = %s",
            usedPixbufModuleFile, usedXDGDataDirs, usedTypelibPath, usedDYLDLibraryPath, usedLuaPath, usedLuaCPath);

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
