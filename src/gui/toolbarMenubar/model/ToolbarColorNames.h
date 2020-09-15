/*
 * Xournal++
 *
 * Names for the toolbar color items (e.g. 0xff000 is called red)
 * Singleton
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <unordered_map>

#include "util/Color.h"

#include "filesystem.h"

class ToolbarColorNames final {
    explicit ToolbarColorNames(fs::path path);

public:
    static ToolbarColorNames& getInstance();

    ~ToolbarColorNames() noexcept;
    ToolbarColorNames(ToolbarColorNames const&) = delete;
    ToolbarColorNames(ToolbarColorNames&&) = delete;
    ToolbarColorNames& operator=(ToolbarColorNames const&) = delete;
    ToolbarColorNames& operator=(ToolbarColorNames&&) = delete;

public:
    void addColor(Color color, const std::string& name, bool predefined);
    std::string getColorName(Color color);

    void load();
    void save() const noexcept;

private:
    void initPredefinedColors();

private:
    GKeyFile* config;
    std::unordered_map<Color, std::string> predefinedColorNames{};
    fs::path file;
};
