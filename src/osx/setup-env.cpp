#include "setup-env.h"

#include <string>

#include <CoreFoundation/CoreFoundation.h>
#include <glib.h>

#include "util/Stacktrace.h"

#include "filesystem.h"

namespace {
/// Set the environment variable name with the path value, if the path exists, and
/// log whether successful or not.
void setenvPathIfExists(const char* name, fs::path const& path, bool override) {
    if (fs::exists(path)) {
        setenv(name, path.c_str(), override);
        std::cerr << "Overriding env var " << name << ": " << path << "\n";
    } else {
        std::cerr << "Skip override env var " << name << ", path doesn't exist: " << path << "\n";
    }
}

}  // namespace

void setupEnvironment() {
    /**
     * see https://gitlab.gnome.org/GNOME/gtk-mac-bundler/-/blob/master/examples/gtk3-launcher.sh
     * and https://gitlab.gnome.org/GNOME/gtk-mac-bundler/-/issues/12
     */

    // Get the parent of the folder containing the application executable
    auto base = Stacktrace::getExePath().parent_path();  // Xournal++.app/Contents or $HOME/gtk/inst

    /* We consider the application to launch from a bundle iff the parent
     * directory is named "Contents" and it has a "Resources" folder.
     * Note that checking the latter only is insufficient, e.g. it will falsely
     * detect a bundle from the build folder, since the repository root has a
     * folder named "resources"
     */
    const bool isInBundle = base.filename() == "Contents" && fs::exists(base / "Resources");
    if (isInBundle) {
        // app-bundle, e.g. Xournal++.app/Contents
        // Install prefix is actually the Resources folder
        base = base / "Resources";
        std::cerr << "xournalpp is running from a bundle\n";
    } else {
        // Assume the "base dir" is the install prefix, corresponding to
        // e.g., build folder or $HOME/gtk/inst
        std::cerr << "xournalpp is running from some non-bundle path\n";
    }
    // Now base should be set to the install prefix

    auto libPath = base / "lib";
    auto dataPath = base / "share";
    auto xdgPath = base / "etc" / "xdg";

    auto imModuleFile = libPath / "gtk-3.0" / "3.0.0" / "immodules.cache";
    auto pixbufModuleFile = libPath / "gdk-pixbuf-2.0" / "2.10.0" / "loaders.cache";

    // Override any environment variables corresponding to install prefix paths.
    // Only do this if the paths don't exist, otherwise we could run into issues
    // (e.g., nonexistent pixbuf module file will lead to a startup crash)
    setenvPathIfExists("XDG_CONFIG_DIRS", xdgPath, 1);
    setenvPathIfExists("XDG_DATA_DIRS", dataPath, 1);
    setenvPathIfExists("GTK_IM_MODULE_FILE", imModuleFile, 1);
    setenvPathIfExists("GDK_PIXBUF_MODULE_FILE", pixbufModuleFile, 1);

    // Only set GTK env vars if the corresponding GTK folder exists.
    if (fs::exists(base / "lib" / "gtk-3.0")) {
        setenvPathIfExists("GTK_EXE_PREFIX", base, 1);
        setenvPathIfExists("GTK_PATH", base, 1);
    }
    if (fs::exists(base / "etc" / "gtk-3.0")) {
        setenvPathIfExists("GTK_DATA_PREFIX", base, 1);
    }

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
                lang.replace(lang.find("-"), 1, "_");  // e.g. "de_DE"
                lang += ".UTF-8";                      // e.g. "de_DE.UTF-8"
                g_message("Setting LANG and LC_MESSAGES to %s", lang.c_str());
                setenv("LANG", lang.c_str(), 0);
                setenv("LC_MESSAGES", lang.c_str(), 0);
            }
        }
    }
}
