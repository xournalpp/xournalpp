/*
 * Xournal++
 *
 * Toolbar definitions model
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include <glib.h>  // for GKeyFile

class ToolbarEntry;


class ToolbarData {
public:
    ToolbarData(bool predefined);
    ToolbarData(const ToolbarData& data);
    virtual ~ToolbarData();

    void operator=(const ToolbarData& other);

public:
    const std::string& getName() const;
    void setName(std::string name);
    const std::string& getId() const;
    void setId(std::string id);
    bool isPredefined() const;

    void load(GKeyFile* config, const char* group);
    void saveToKeyFile(GKeyFile* config);

    // Editing API
    int insertItem(const std::string& toolbar, const std::string& item, int position);
    bool removeItemByID(const std::string& toolbar, int id);

private:
    std::string id;
    std::string name;
    std::vector<ToolbarEntry*> contents;

    bool predefined = false;

    friend class ToolbarModel;
    friend class ToolMenuHandler;
};
