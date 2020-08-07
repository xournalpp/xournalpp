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
#include <vector>

#include "XournalType.h"
#include "filesystem.h"

class ToolbarColorNames {
private:
    ToolbarColorNames();
    virtual ~ToolbarColorNames();

public:
    static ToolbarColorNames& getInstance();
    static void freeInstance();

public:
    void loadFile(fs::path const& file);
    void saveFile(fs::path const& file);

    void addColor(int color, const string& name, bool predefined);

    string getColorName(int color);

private:
    void initPredefinedColors();

private:
    GKeyFile* config;
    GHashTable* predefinedColorNames;
};
