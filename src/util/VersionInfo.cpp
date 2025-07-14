#include "util/VersionInfo.h"

#include <sstream>

#include <glib.h>
#include <gtk/gtk.h>

#include "util/raii/CStringWrapper.h"

#include "config-git.h"
#include "config.h"

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
static bool isX11() { return GDK_IS_X11_DISPLAY(gdk_display_get_default()); }
#else
static bool isX11() { return false; }
#endif

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
static bool isWayland() { return GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default()); }
#else
static bool isWayland() { return false; }
#endif

#ifdef GDK_WINDOWING_QUARTZ
#include <gdk/gdkquartz.h>
static bool isQuartz() { return GDK_IS_QUARTZ_DISPLAY(gdk_display_get_default()); }
#else
static bool isQuartz() { return false; }
#endif

#ifdef GDK_WINDOWING_BROADWAY
#include <gdk/gdkbroadway.h>
static bool isBroadway() { return GDK_IS_BROADWAY_DISPLAY(gdk_display_get_default()); }
#else
static bool isBroadway() { return false; }
#endif

#ifdef GDK_WINDOWING_WIN32
#include <gdk/gdkwin32.h>
static bool isWin32() { return GDK_IS_WIN32_DISPLAY(gdk_display_get_default()); }
#else
static bool isWin32() { return false; }
#endif

namespace xoj::util {
const char* getGdkBackend() {
    if (gdk_display_get_default()) {
        if (isX11()) {
            return "X11";
        } else if (isWayland()) {
            return "Wayland";
        } else if (isBroadway()) {
            return "Broadway";
        } else if (isQuartz()) {
            return "Quartz";
        } else if (isWin32()) {
            return "Win32";
        } else {
            return "Unknown GDK backend!!";
        }
    } else {
        return nullptr;
    }
}

std::string getXournalppVersion() {
    auto str = std::string(PROJECT_NAME) + " " + PROJECT_VERSION;
    if (!std::string(GIT_COMMIT_ID).empty()) {
        str = str + " (" + GIT_COMMIT_ID + " from " + GIT_ORIGIN_OWNER + "/" + GIT_BRANCH + ")";
    }
    return str;
}

std::string getOsInfo() {
    auto osInfo = xoj::util::OwnedCString::assumeOwnership(g_get_os_info(G_OS_INFO_KEY_NAME));
    if (!osInfo) {
        osInfo = xoj::util::OwnedCString::assumeOwnership(g_get_os_info(G_OS_INFO_KEY_PRETTY_NAME));
    }
    if (osInfo) {
        xoj::util::OwnedCString osVersion;
        for (auto key: {G_OS_INFO_KEY_VERSION, G_OS_INFO_KEY_VERSION_ID, G_OS_INFO_KEY_VERSION_CODENAME}) {
            osVersion = xoj::util::OwnedCString::assumeOwnership(g_get_os_info(key));
            if (osVersion) {
                break;
            }
        }
        if (osVersion) {
            return std::string(osInfo.get()) + " " + osVersion.get();
        } else {
            return std::string(osInfo.get());
        }
    }
    return std::string();
}


std::string getVersionInfo() {
    std::stringstream str;
    str.imbue(std::locale::classic());

    str << getXournalppVersion() << std::endl;

    str << "├──libgtk: " << gtk_get_major_version() << "." << gtk_get_minor_version() << "." << gtk_get_micro_version()
        << std::endl;
    str << "├──glib: " << glib_major_version << "." << glib_minor_version << "." << glib_micro_version << std::endl;
    str << "├──cairo:  " << cairo_version_string() << std::endl;


    if (const char* backend = getGdkBackend(); backend) {
        str << "├──GDK backend: " << backend << std::endl;
    }

    str << "└──OS info: " << getOsInfo() << std::endl;

    return str.str();
}
};  // namespace xoj::util
