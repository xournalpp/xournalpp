// FIXME Guard idea from GPL?

#pragma once

#include <optional>

#include "Actions.h"

class PlatformConsole {
public:
    PlatformConsole() = default;
    PlatformConsole(const PlatformConsole&) = delete;
    PlatformConsole(PlatformConsole&&) = delete;
    PlatformConsole& operator=(const PlatformConsole&) = delete;
    PlatformConsole& operator=(PlatformConsole&&) = delete;
    virtual ~PlatformConsole() = default;

    virtual void show(bool flash) = 0;
    virtual void showUnconditionally(bool flash) = 0;
    virtual void hide() = 0;
    virtual void sendHeartbeat() = 0;
    virtual void setShowOnDestruction(bool) = 0;
};

class ConsoleCtl final: public ActionHandler {
public:
    ConsoleCtl() = default;
    ConsoleCtl(const ConsoleCtl&) = delete;
    ConsoleCtl(ConsoleCtl&&) = delete;
    ConsoleCtl& operator=(const ConsoleCtl&) = delete;
    ConsoleCtl& operator=(ConsoleCtl&&) = delete;
    ~ConsoleCtl() = default;

    auto hasPlatformConsole() -> bool;

    /**
     * Show console window if it has meaningful output (i.e. at least one of stdout, stderr is directed there)
     */
    void show(bool flash);
    /**
     * Show console window even if it isn't used because both stdout and stderr are redirected
     */
    void showUnconditionally(bool flash);
    void hide();
    void sendHeartbeat();
    void setShowOnExit(bool);

    /**
     * Doesn't handle any actions yet
     */
    void actionPerformed(ActionType type, ActionGroup group, GdkEvent* event, GtkMenuItem* menuitem,
                         GtkToolButton* toolbutton, bool enabled) override;

    /**
     * To be implemented per platform
     */
    static void initPlatformConsole(bool show);
    /**
     * To be implemented per platform
     *
     * @return std::optional<PlatformConsole>&
     */
    [[nodiscard]] static auto getPlatformConsole() -> std::optional<PlatformConsole*>;
    /**
     * To be implemented per platform
     */
    static void abnormalExit();

private:
    std::optional<PlatformConsole*> platformConsole_{getPlatformConsole()};
};
