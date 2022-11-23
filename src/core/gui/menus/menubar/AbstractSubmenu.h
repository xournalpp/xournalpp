/*
 * Xournal++
 *
 * A submenu
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/raii/GObjectSPtr.h"

class MainWindow;

class Submenu {
public:
    Submenu() = default;
    virtual ~Submenu() noexcept = default;

public:
    virtual void setDisabled(bool disabled) = 0;
    virtual void addToMenubar(MainWindow* win) = 0;

protected:
    xoj::util::WidgetSPtr menuItem;
};
