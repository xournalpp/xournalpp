/*
 * Xournal++
 *
 * Handles different page background types
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

#include <glib.h>

#include "model/PageType.h"

#include "filesystem.h"

class PageTypeInfo {
public:
    PageType page;
    std::string name;
};

class GladeSearchpath;

class PageTypeHandler {
public:
    PageTypeHandler(GladeSearchpath* gladeSearchPath);
    virtual ~PageTypeHandler();

public:
    std::vector<PageTypeInfo*>& getPageTypes();
    static PageTypeFormat getPageTypeFormatForString(const std::string& format);
    static std::string getStringForPageTypeFormat(const PageTypeFormat& format);

private:
    void addPageTypeInfo(std::string name, PageTypeFormat format, std::string config);
    bool parseIni(fs::path const& filepath);
    void loadFormat(GKeyFile* config, const char* group);

private:
    std::vector<PageTypeInfo*> types;
};
