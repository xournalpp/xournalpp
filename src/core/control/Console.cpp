#include "Console.h"

#include <type_traits>

#ifdef _WIN32
#include "ConsoleWindows.h"
#else
#include "ConsoleUnix.h"
#endif


auto ConsoleCtl::hasPlatformConsole() -> bool { return platformConsole_.has_value(); }

void ConsoleCtl::show(bool flash) {
    if (platformConsole_)
        platformConsole_.value()->show(flash);
}
void ConsoleCtl::showUnconditionally(bool flash) {
    if (platformConsole_)
        platformConsole_.value()->showUnconditionally(flash);
}
void ConsoleCtl::hide() {
    if (platformConsole_)
        platformConsole_.value()->hide();
}
void ConsoleCtl::setShowOnExit(bool show) {
    if (platformConsole_)
        platformConsole_.value()->setShowOnDestruction(show);
}
void ConsoleCtl::sendHeartbeat() {
    if (platformConsole_)
        platformConsole_.value()->sendHeartbeat();
}

void ConsoleCtl::actionPerformed(ActionType type, ActionGroup group, GdkEvent* event, GtkMenuItem* menuitem,
                                 GtkToolButton* toolbutton, bool enabled) {}
