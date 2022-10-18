#include "setup-env.h"

#include <string>

#include <CoreFoundation/CoreFoundation.h>
#include <glib.h>

#include "util/Stacktrace.h"

#include "filesystem.h"


void setupEnvironment() {
    /**
     * see https://gitlab.gnome.org/GNOME/gtk-mac-bundler/-/blob/master/examples/gtk3-launcher.sh
     * and https://gitlab.gnome.org/GNOME/gtk-mac-bundler/-/issues/12
     */
    auto base = Stacktrace::getExePath().parent_path();  // Xournal++.app/Contents or $HOME/gtk/inst

    if (fs::exists(base / "Resources")) {  // app-bundle
        base = base / "Resources";
    }  // Now base is Xournal++.app/Contents/Resources or $HOME/gtk/inst

    auto libPath = base / "lib";
    auto dataPath = base / "share";
    auto xdgPath = base / "etc" / "xdg";

    auto imModuleFile = libPath / "gtk-3.0" / "3.0.0" / "immodules.cache";
    auto pixbufModuleFile = libPath / "gdk-pixbuf-2.0" / "2.10.0" / "loaders.cache";

    setenv("XDG_CONFIG_DIRS", xdgPath.string().c_str(), 1);
    setenv("XDG_DATA_DIRS", dataPath.string().c_str(), 1);
    setenv("GTK_DATA_PREFIX", base.string().c_str(), 1);
    setenv("GTK_EXE_PREFIX", base.string().c_str(), 1);
    setenv("GTK_PATH", base.string().c_str(), 1);

    setenv("GTK_IM_MODULE_FILE", imModuleFile.string().c_str(), 0);
    setenv("GDK_PIXBUF_MODULE_FILE", pixbufModuleFile.string().c_str(), 0);

    auto environ = g_get_environ();
    const char* usedPixbufModuleFile = g_environ_getenv(environ, "GDK_PIXBUF_MODULE_FILE");
    g_message("Continue with GDK_PIXBUF_MODULE_FILE = %s", usedPixbufModuleFile);

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
                std::string lang = buffer;             // e.g. "de-DE"
                if (auto pos = lang.find("-"); pos != std::string::npos) {
                    lang.replace(pos, 1, "_");  // e.g. "de_DE"
                }
                lang += ".UTF-8";                      // e.g. "de_DE.UTF-8"
                g_message("Setting LANG and LC_MESSAGES to %s", lang.c_str());
                setenv("LANG", lang.c_str(), 0);
                setenv("LC_MESSAGES", lang.c_str(), 0);
            }
        }
    }
}
