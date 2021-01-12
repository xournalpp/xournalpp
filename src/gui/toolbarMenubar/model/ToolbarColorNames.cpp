#include "ToolbarColorNames.h"

#include <array>
#include <cinttypes>
#include <fstream>
#include <memory>

#include <glib/gstdio.h>
#include <util/PathUtil.h>

#include "i18n.h"

namespace {

void load(fs::path const& file, GKeyFile* config) {
    GError* error = nullptr;
    if (!g_key_file_load_from_file(config, file.u8string().c_str(), G_KEY_FILE_NONE, &error)) {
        g_warning("Failed to load \"colornames.ini\" (%s): %s\n", file.string().c_str(), error->message);
        g_error_free(error);
        return;
    }
    g_key_file_set_string(config, "info", "about", "Xournalpp custom color names");
}

void save(fs::path const& file, GKeyFile* config) noexcept {
    gsize len = 0;
    char* data = g_key_file_to_data(config, &len, nullptr);
    try {
        std::ofstream ofs{file, std::ios::binary};
        ofs.exceptions(std::ios::badbit | std::ios::failbit);
        ofs.write(data, len);
    } catch (std::ios_base::failure const& e) {
        g_warning("Could not save color file \"%s\".\n%s with error code %d", file.u8string().c_str(), e.what(),
                  e.code().value());
    }
    g_free(data);
}

}  // namespace

ToolbarColorNames::ToolbarColorNames(fs::path readFile): file(std::move(readFile)) {
    this->config = g_key_file_new();
    g_key_file_set_string(this->config, "info", "about", "Xournalpp custom color names");
    initPredefinedColors();
    load();
}

ToolbarColorNames::~ToolbarColorNames() noexcept { g_key_file_free(this->config); }

auto ToolbarColorNames::getInstance() -> ToolbarColorNames& {
    static std::unique_ptr<ToolbarColorNames> instance{new ToolbarColorNames(Util::getConfigFile("colornames.ini"))};
    return *instance;
}

void ToolbarColorNames::load() { ::load(this->file, this->config); }

void ToolbarColorNames::save() const noexcept { ::save(this->file, this->config); }

void ToolbarColorNames::addColor(Color color, const std::string& name, bool predefined) {
    if (predefined) {
        this->predefinedColorNames[color] = name;
    } else {
        std::array<char, 16> colorHex{};
        snprintf(colorHex.data(), colorHex.size(), "%06" PRIx32, uint32_t{color});
        g_key_file_set_string(this->config, "custom", colorHex.data(), name.c_str());
    }
}

auto ToolbarColorNames::getColorName(Color color) -> std::string {
    std::array<char, 16> colorHex{};
    snprintf(colorHex.data(), colorHex.size(), "%06" PRIx32, uint32_t{color});

    std::string colorName;
    char* name = g_key_file_get_string(this->config, "custom", colorHex.data(), nullptr);
    if (name != nullptr) {
        colorName = name;
        g_free(name);
    }

    if (!colorName.empty()) {
        return colorName;
    }

    if (auto iter = this->predefinedColorNames.find(color); iter != end(this->predefinedColorNames)) {
        return iter->second;
    }

    return {colorHex.data(), colorHex.size()};
}

void ToolbarColorNames::initPredefinedColors() {
    // Here you can add predefined color names
    // this ordering fixes #2
    addColor(0x000000U, _("Black"), true);
    addColor(0x008000U, _("Green"), true);
    addColor(0x00c0ffU, _("Light Blue"), true);
    addColor(0x00ff00U, _("Light Green"), true);
    addColor(0x3333ccU, _("Blue"), true);
    addColor(0x808080U, _("Gray"), true);
    addColor(0xff0000U, _("Red"), true);
    addColor(0xff00ffU, _("Magenta"), true);
    addColor(0xff8000U, _("Orange"), true);
    addColor(0xffff00U, _("Yellow"), true);
    addColor(0xffffffU, _("White"), true);
}
