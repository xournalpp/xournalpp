#include "ToolbarData.h"

#include <algorithm>
#include <cstring>  // for strcmp, strncmp
#include <utility>  // for move

#include "gui/toolbarMenubar/model/ColorPalette.h"  // for Palette
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
    this->contents = other.contents;
}

ToolbarData::~ToolbarData() = default;

auto ToolbarData::getName() const -> const std::string& { return this->name; }

void ToolbarData::setName(string name) { this->name = std::move(name); }

auto ToolbarData::getId() const -> const std::string& { return this->id; }

void ToolbarData::setId(string id) { this->id = std::move(id); }

auto ToolbarData::isPredefined() const -> bool { return this->predefined; }

void ToolbarData::load(GKeyFile* config, const char* group, const Palette& colorPalette) {

    size_t unassignedColorIndex = 0U;
    auto filterDeprecatedItemNames = [&](std::string name) -> std::string {
        // recognize previous name, V1.07 (Jan 2019) and earlier.
        if (name == "TWO_PAGES") {
            return "PAIRED_PAGES";
        }

        // recognize previous name, V1.08 (Feb 2019) and earlier.
        if (name == "RECSTOP") {
            return "AUDIO_RECORDING";
        }

        // recognize previous name, V1.0.19 (Dec 2020) and earlier
        if (name == "HILIGHTER") {
            return "HIGHLIGHTER";
        }

        // recognize previous name, V1.1.0+dev (Jan 2021) and earlier
        if (name == "DRAW_CIRCLE") {
            return "DRAW_ELLIPSE";
        }

        // recognize previous name, V1.1.0+dev (Nov 2022) and earlier
        if (name == "PEN_FILL_OPACITY") {
            return "FILL_OPACITY";
        }

        if (StringUtils::startsWith(name, "COLOR(") && StringUtils::endsWith(name, ")")) {
            std::string arg = name.substr(6, name.length() - 7);
            // check for old color format of toolbar.ini
            if (StringUtils::startsWith(arg, "0x")) {
                auto paletteIndex = colorPalette.getColorAt(unassignedColorIndex++).getIndex();
                return "COLOR(" + std::to_string(paletteIndex) + ")";
            }
        }

        return name;
    };

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

        auto& e = contents.emplace_back();
        e.setName(keys[i]);

        gsize keyLen = 0;
        gchar** list = g_key_file_get_string_list(config, group, keys[i], &keyLen, nullptr);

        for (gsize x = 0; x < keyLen; x++) {
            e.addItem(filterDeprecatedItemNames(StringUtils::trim(std::string(list[x]))));
        }

        g_strfreev(list);
    }

    g_strfreev(keys);
}

void ToolbarData::saveToKeyFile(GKeyFile* config) const {
    string group = getId();

    for (const ToolbarEntry& e: this->contents) {
        string line{};

        for (const auto& it: e.getItems()) {
            line += ",";
            line += it.getName();
        }

        if (line.length() > 2) {
            g_key_file_set_string(config, group.c_str(), e.getName().c_str(), line.substr(1).c_str());
        }
    }

    g_key_file_set_string(config, group.c_str(), "name", this->name.c_str());
}

void ToolbarData::setEntry(ToolbarEntry entry) {
    auto it = std::find_if(contents.begin(), contents.end(), [&](auto&& e) { return e.getName() == entry.getName(); });
    if (it == contents.end()) {
        // Empty toolbars don't need an entry
        if (!entry.getItems().empty()) {
            contents.emplace_back(std::move(entry));
        }
    } else {
        if (entry.getItems().empty()) {
            // Empty toolbars don't need an entry
            contents.erase(it);
        } else {
            *it = std::move(entry);
        }
    }
}
