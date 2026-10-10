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
struct Palette;

/// Corresponds to one configuration, containing all the various ToolbarBoxes (around the document or floating)
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

    /// Adds the ToolbarEntry to the configuration. Overrides any preexisting entry with the same name.
    void setEntry(ToolbarEntry e);

    void load(GKeyFile* config, const char* group, const Palette& colorPalette);
    void saveToKeyFile(GKeyFile* config) const;

private:
    std::string id;
    std::string name;
    std::vector<ToolbarEntry> contents;

    bool predefined = false;

    friend class ToolbarModel;
    friend class ToolMenuHandler;
};
