#include "ToolbarData.h"

#include <cstring>  // for strcmp, strncmp
#include <utility>  // for move

#include "gui/toolbarMenubar/model/ToolbarEntry.h"  // for ToolbarEntry, Too...
#include "gui/toolbarMenubar/model/ToolbarItem.h"   // for ToolbarItem
#include "util/PlaceholderString.h"                 // for PlaceholderString
#include "util/StringUtils.h"                       // for StringUtils
#include "util/i18n.h"                              // for FC, FORMAT_STR, _F

using std::string;

ToolbarData::ToolbarData(bool predefined): predefined(predefined) {}

ToolbarData::ToolbarData(const ToolbarData& data) {
    *this = data;
    this->predefined = false;
}

void ToolbarData::operator=(const ToolbarData& other) {
    this->id = other.id;
    this->name = other.name;
    this->predefined = other.predefined;

    contents.clear();
    for (const ToolbarEntry* e: other.contents) { contents.push_back(new ToolbarEntry(*e)); }
}

ToolbarData::~ToolbarData() {
    for (ToolbarEntry* e: this->contents) { delete e; }
    contents.clear();
}

auto ToolbarData::getName() const -> const std::string& { return this->name; }

void ToolbarData::setName(string name) { this->name = std::move(name); }

auto ToolbarData::getId() const -> const std::string& { return this->id; }

void ToolbarData::setId(string id) { this->id = std::move(id); }

auto ToolbarData::isPredefined() const -> bool { return this->predefined; }

void ToolbarData::load(GKeyFile* config, const char* group) {
    gsize length = 0;
    gchar** keys = g_key_file_get_keys(config, group, &length, nullptr);
    if (keys == nullptr) {
        return;
    }

    gchar* name = g_key_file_get_locale_string(config, group, "name", nullptr, nullptr);
    if (name != nullptr) {
        this->name = name;
        g_free(name);
    }

    for (gsize i = 0; i < length; i++) {
        if (strcmp(keys[i], "name") == 0 || strncmp(keys[i], "name[", 5) == 0) {
            continue;
        }

        auto* e = new ToolbarEntry();
        gsize keyLen = 0;
        e->setName(keys[i]);
        gchar** list = g_key_file_get_string_list(config, group, keys[i], &keyLen, nullptr);

        for (gsize x = 0; x < keyLen; x++) { e->addItem(StringUtils::trim(string(list[x]))); }

        contents.push_back(e);

        g_strfreev(list);
    }

    g_strfreev(keys);
}

void ToolbarData::saveToKeyFile(GKeyFile* config) {
    string group = getId();

    for (ToolbarEntry* e: this->contents) {
        string line{};

        for (ToolbarItem* it: e->getItems()) {
            line += ",";
            line += it->getName();
        }

        if (line.length() > 2) {
            g_key_file_set_string(config, group.c_str(), e->getName().c_str(), line.substr(1).c_str());
        }
    }

    g_key_file_set_string(config, group.c_str(), "name", this->name.c_str());
}

auto ToolbarData::insertItem(const string& toolbar, const string& item, int position) -> int {
    g_message("%s", FC(FORMAT_STR("ToolbarData::insertItem({1}, {2}, {3});") % toolbar % item % position));

    g_return_val_if_fail(isPredefined() == false, -1);

    for (ToolbarEntry* e: this->contents) {
        if (e->getName() == toolbar) {
            g_message("%s", FC(_F("Toolbar found: {1}") % toolbar));

            int id = e->insertItem(item, position);

            g_message("%s", FC(FORMAT_STR("return {1}") % id));
            return id;
        }
    }

    auto* newEntry = new ToolbarEntry();
    newEntry->setName(toolbar);
    int id = newEntry->addItem(item);
    contents.push_back(newEntry);

    return id;
}

auto ToolbarData::removeItemByID(const string& toolbar, int id) -> bool {
    g_return_val_if_fail(isPredefined() == false, false);

    for (ToolbarEntry* e: contents) {
        if (e->getName() == toolbar) {
            return e->removeItemById(id);
        }
    }

    return false;
}
