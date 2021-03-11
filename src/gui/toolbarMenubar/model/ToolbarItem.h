/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>


class ToolbarItem {
public:
    ToolbarItem(std::string name);
    ToolbarItem();

public:
    const std::string& getName() const;

    bool operator==(ToolbarItem& other);

    int getId() const;

private:
    std::string name;
    int id;

    static int sid;
};
