#include "PageTemplateSettings.h"

#include <cinttypes>  // for PRIx32
#include <cstdint>    // for uint32_t
#include <cstdio>     // for snprintf, size_t
#include <sstream>    // for basic_istream, strings...

#include "control/pagetype/PageTypeHandler.h"  // for PageTypeHandler

using std::string;
using std::stringstream;

PageTemplateSettings::PageTemplateSettings():
        copyLastPageSettings(true),
        copyLastPageSize(false),
        pageWidth(595.275591),
        pageHeight(841.889764),
        backgroundColor(Colors::white) {
    backgroundType.format = PageTypeFormat::Lined;
}

PageTemplateSettings::~PageTemplateSettings() = default;

auto PageTemplateSettings::isCopyLastPageSettings() const -> bool { return this->copyLastPageSettings; }

void PageTemplateSettings::setCopyLastPageSettings(bool copyLastPageSettings) {
    this->copyLastPageSettings = copyLastPageSettings;
}

auto PageTemplateSettings::isCopyLastPageSize() const -> bool { return this->copyLastPageSize; }

void PageTemplateSettings::setCopyLastPageSize(bool copyLastPageSize) { this->copyLastPageSize = copyLastPageSize; }

auto PageTemplateSettings::getPageWidth() const -> double { return this->pageWidth; }

void PageTemplateSettings::setPageWidth(double pageWidth) { this->pageWidth = pageWidth; }

auto PageTemplateSettings::getPageHeight() const -> double { return this->pageHeight; }

void PageTemplateSettings::setPageHeight(double pageHeight) { this->pageHeight = pageHeight; }

auto PageTemplateSettings::getBackgroundColor() const -> Color { return this->backgroundColor; }

void PageTemplateSettings::setBackgroundColor(Color backgroundColor) { this->backgroundColor = backgroundColor; }

auto PageTemplateSettings::getBackgroundType() -> PageType { return backgroundType; }

auto PageTemplateSettings::getPageInsertType() -> PageType {
    if (copyLastPageSettings) {
        return PageType(PageTypeFormat::Copy);
    }

    return backgroundType;
}

void PageTemplateSettings::setBackgroundType(const PageType& backgroundType) { this->backgroundType = backgroundType; }

/**
 * Parse a template string
 *
 * @return true if valid
 */
auto PageTemplateSettings::parse(const std::string& tpl) -> bool {
    stringstream ss(tpl);
    string line;

    if (!std::getline(ss, line, '\n')) {
        return false;
    }

    if (line != "xoj/template") {
        return false;
    }

    while (std::getline(ss, line, '\n')) {
        size_t pos = line.find('=');
        if (pos == string::npos) {
            continue;
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        if (key == "copyLastPageSettings") {
            copyLastPageSettings = value == "true";
        } else if (key == "copyLastPageSize") {
            copyLastPageSize = value == "true";
        } else if (key == "size") {
            pos = value.find('x');
            pageWidth = std::stod(value.substr(0, pos));
            pageHeight = std::stod(value.substr(pos + 1));
        } else if (key == "backgroundColor") {
            backgroundColor = Color(std::stoul(value.substr(1), nullptr, 16));
        } else if (key == "backgroundType") {
            this->backgroundType.format = PageTypeHandler::getPageTypeFormatForString(value);
        } else if (key == "backgroundTypeConfig") {
            this->backgroundType.config = value;
        }
    }

    return true;
}

/**
 * Convert to a parsable string
 */
auto PageTemplateSettings::toString() const -> string {
    string str = "xoj/template\n";

    str += string("copyLastPageSize=") + (copyLastPageSize ? "true" : "false") + "\n";
    str += string("copyLastPageSettings=") + (copyLastPageSettings ? "true" : "false") + "\n";
    str += string("size=") + std::to_string(pageWidth) + "x" + std::to_string(pageHeight) + "\n";
    str += string("backgroundType=") + PageTypeHandler::getStringForPageTypeFormat(backgroundType.format) + "\n";

    if (!backgroundType.config.empty()) {
        str += string("backgroundTypeConfig=") + backgroundType.config + "\n";
    }

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "#%06" PRIx32, uint32_t{this->backgroundColor});
    str += string("backgroundColor=") + buffer + "\n";

    return str;
}
