/*
 * Xournal++
 *
 * Handles differnt page background types
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "model/PageType.h"

#include "XournalType.h"
#include "filesystem.h"

class PageTypeInfo {
public:
    PageType page;
    string name;
};

class GladeSearchpath;

class PageTypeHandler {
public:
    PageTypeHandler(GladeSearchpath* gladeSearchPath);
    virtual ~PageTypeHandler();

public:
    vector<PageTypeInfo*>& getPageTypes();
    static PageTypeFormat getPageTypeFormatForString(const string& format);
    static string getStringForPageTypeFormat(const PageTypeFormat& format);

private:
    void addPageTypeInfo(string name, PageTypeFormat format, string config);
    bool parseIni(fs::path const& filepath);
    void loadFormat(GKeyFile* config, const char* group);

private:
    vector<PageTypeInfo*> types;
};
