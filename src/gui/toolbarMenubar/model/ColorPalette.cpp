#include "ColorPalette.h"

#include <fstream>
#include <limits>
#include <sstream>

#include <gtk/gtk.h>

#include "util/StringUtils.h"

Palette::Palette(fs::path path): filepath{std::move(path)}, header{}, namedColors{} {}

void Palette::load() {
    header.clear();
    namedColors.clear();
    try {
        std::ifstream gptFile{filepath};
        std::string line;

        if (gptFile.is_open()) {
            getline(gptFile, line);
            if (!parseFirstGimpPaletteLine(line))
                throw std::invalid_argument("This is not a gpt file, using default palette instead.");
            while (getline(gptFile, line)) {
                parseHeaderLine(line) || parseColorLine(line);
            }
            if (namedColors.size() < 1)
                throw std::invalid_argument("Your Palettefile has no parsable color. It needs at least one!");
        }
    } catch (const std::exception& e) {
        g_warning("There has been a problem parsing the palette.gpl file at %s. \n"
                  "What happened: %s \n"
                  "What to do: %s \n",
                  filepath.c_str(), e.what(),
                  "Please fix your palette file, or rename it so xournalpp creates a new default one for you which you "
                  "can use as a template.");

        // Parse Default Palette string instead
        // This is not computationally efficient but this ways there is only one place (default_palette()) where the
        // palette is defined.
        std::stringstream defaultFile{default_palette()};
        std::string line;

        getline(defaultFile, line);
        if (!parseFirstGimpPaletteLine(line))
            g_error("The default file was mallformed. This should never happen!");
        while (getline(defaultFile, line)) {
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
        namedColors.emplace_back(std::move(color));
        return true;
    }
    return false;
}

NamedColor const& Palette::getColorAt(size_t i) const {
    if (i >= namedColors.size()) {
        i = i % namedColors.size();
        g_warning("There are more Coloritems in the Toolbar then your Palette defines.\n"
                  "Hence, cycling through palette from the beginning.");
    }

    return namedColors.at(i);
}

size_t Palette::size() const { return namedColors.size(); }


const std::string Palette::default_palette() {
    std::stringstream d{};
    d << "GIMP Palette"
      << "\n";
    d << "Name: Xournal Default Palette"
      << "\n";
    d << "#"
      << "\n";
    d << 0 << " " << 0 << " " << 0 << " "
      << "Black"
      << "\n";
    d << 0 << " " << 128 << " " << 0 << " "
      << "Green"
      << "\n";
    d << 0 << " " << 192 << " " << 255 << " "
      << "Light Blue"
      << "\n";
    d << 0 << " " << 255 << " " << 0 << " "
      << "Light Green"
      << "\n";
    d << 51 << " " << 51 << " " << 204 << " "
      << "Blue"
      << "\n";
    d << 128 << " " << 128 << " " << 128 << " "
      << "Gray"
      << "\n";
    d << 255 << " " << 0 << " " << 0 << " "
      << "Red"
      << "\n";
    d << 255 << " " << 0 << " " << 255 << " "
      << "Magenta"
      << "\n";
    d << 255 << " " << 128 << " " << 0 << " "
      << "Orange"
      << "\n";
    d << 255 << " " << 255 << " " << 0 << " "
      << "Yellow"
      << "\n";
    d << 255 << " " << 255 << " " << 255 << " "
      << "White"
      << "\n";
    return d.str();
}

void Palette::create_default(fs::path filepath) {
    std::ofstream myfile{filepath};
    myfile << default_palette();
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
            header = std::move(tmp);
        } else {
            // One operation failed.
            // So set the state on the main stream
            // to indicate failure.
            str.setstate(std::ios::failbit);
        }
    }
    return str;
}
