/*
 * Xournal++
 *
 * Helper which allows to switch between Xournal++ icons and stock icons
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <string>

class Settings;

class IconNameHelper final {

public:
    IconNameHelper(Settings* settings);

protected:
    const Settings* settings;

public:
    std::string iconName(const char* icon) const;
};
