#include "ColorPalette.h"

#include <fstream>
#include <limits>
#include <sstream>

#include <gtk/gtk.h>
#include <util/StringUtils.h>

Palette::Palette(fs::path path): filepath{std::move(path)}, header{}, namedColors{} {}

void Palette::load() {
    header.clear();
    namedColors.clear();

    std::ifstream gptFile{filepath};
    std::string line;

    if (gptFile.is_open()) {
        getline(gptFile, line);
        if (!parseFirstGimpPaletteLine(line))
            return;
        while (getline(gptFile, line)) {
            parseHeaderLine(line) || parseColorLine(line);
        }
    }
}

auto Palette::parseFirstGimpPaletteLine(const std::string& line) -> bool {
    if (StringUtils::trim(line) != "GIMP Palette") {
        g_error(".gpt file needs to start with \"GIMP Palette\" in the "
                "first line");
        return false;
    }
    return true;
}

auto Palette::parseHeaderLine(const std::string& line) -> bool {
    Header headerEntry;
    NamedColor color;
    std::istringstream iss{line};
    if (iss >> headerEntry) {
        header[headerEntry.attribute] = headerEntry.value;
        return true;
    }
    return false;
}

auto Palette::parseColorLine(const std::string& line) -> bool {
    NamedColor color;
    std::istringstream iss{line};
    if (iss >> color) {
        // call to private variable paletteIndex allowed because of friend class
        color.paletteIndex = namedColors.size();
        namedColors.push_back(color);
        return true;
    }
    return false;
}


NamedColor Palette::getColorAt(size_t i) const {
    if (i >= namedColors.size()) {
        i = i % namedColors.size();
        g_warning("There are more Coloritems in the Toolbar then your Palette defines.\n"
                  "Hence, cycling through palette from the beginning.");
    }

    return namedColors.at(i);
}

NamedColor* Palette::getColorPointerAt(size_t i) {
    if (i >= namedColors.size()) {
        i = i % namedColors.size();
        g_warning("There are more Coloritems in the Toolbar then your Palette defines.\n"
                  "Hence, cycling through palette from the beginning.");
    }

    return &namedColors.at(i);
}

size_t Palette::size() const { return namedColors.size(); }


void Palette::create_default(fs::path filepath) {
    std::ofstream myfile{filepath};
    myfile << "GIMP Palette"
           << "\n";
    myfile << "Name: Xournal Default Palette"
           << "\n";
    myfile << "#"
           << "\n";
    myfile << 0 << " " << 0 << " " << 0 << " "
           << "Black"
           << "\n";
    myfile << 0 << " " << 128 << " " << 0 << " "
           << "Green"
           << "\n";
    myfile << 0 << " " << 192 << " " << 255 << " "
           << "Light Blue"
           << "\n";
    myfile << 0 << " " << 255 << " " << 0 << " "
           << "Light Green"
           << "\n";
    myfile << 51 << " " << 51 << " " << 204 << " "
           << "Blue"
           << "\n";
    myfile << 128 << " " << 128 << " " << 128 << " "
           << "Gray"
           << "\n";
    myfile << 255 << " " << 0 << " " << 0 << " "
           << "Red"
           << "\n";
    myfile << 255 << " " << 0 << " " << 255 << " "
           << "Magenta"
           << "\n";
    myfile << 255 << " " << 128 << " " << 0 << " "
           << "Orange"
           << "\n";
    myfile << 255 << " " << 255 << " " << 0 << " "
           << "Yellow"
           << "\n";
    myfile << 255 << " " << 255 << " " << 255 << " "
           << "White"
           << "\n";
}

std::istream& operator>>(std::istream& str, Header& header) {
    // inspired by
    // https://codereview.stackexchange.com/questions/38879/parsing-text-file-in-c
    std::string line;
    Header tmp;
    if (std::getline(str, line)) {
        std::istringstream iss{line};
        if (getline(iss, tmp.attribute, ':') && getline(iss, tmp.value, '\n')) {
            tmp.attribute = StringUtils::trim(tmp.attribute);
            tmp.value = StringUtils::trim(tmp.value);

            // parsing was successful hence results can be stored permanently
            header.swap(tmp);
        } else {
            // One operation failed.
            // So set the state on the main stream
            // to indicate failure.
            str.setstate(std::ios::failbit);
        }
    }
    return str;
}

void Header::swap(Header& other) {
    std::swap(attribute, other.attribute);
    std::swap(value, other.value);
}
