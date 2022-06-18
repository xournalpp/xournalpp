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

#include <string>  // for string


class ToolbarItem {
public:
    ToolbarItem(std::string name);
    ToolbarItem();

public:
    std::string getName() const;
    void setName(std::string name);

    bool operator==(ToolbarItem& other);

    int getId() const;

private:
    /**
     * @brief This is the name of the ToolbarItem
     * This is also what is stored in the toolbar.ini verbatim.
     *
     */
    std::string name;
    int id;

    static int sid;
};
