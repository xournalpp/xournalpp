#include "ColorPalette.h"

#include <fstream>
#include <limits>
#include <sstream>

#include <gtk/gtk.h>
#include <util/StringUtils.h>

Palette::Palette(fs::path path): filepath{std::move(path)}, header{}, colors{}, next{0} {}

void Palette::load() {
    std::ifstream myfile{filepath};
    std::string line;
    Header h;
    if (myfile.is_open()) {
        getline(myfile, line);
        line = StringUtils::trim(line);
        if (line != "GIMP Palette") {
            g_error(".gpt file needs to start with \"GIMP Palette\" in the "
                    "first line");
        }
        while (getline(myfile, line)) {
            std::istringstream iss{line};
            if (iss >> h) {
                header[h.attribute] = h.value;
            } else {
                NamedColor color;
                std::istringstream iss2{line};
                if (iss2 >> color) {
                    color.paletteIndex = colors.size();
                    colors.push_back(color);
                }
            }
        }
    }
}

NamedColor Palette::getNext(int inc) {
    NamedColor result = colors.at(next);
    next += inc;
    if (next < 0) {
        next = colors.size() - 1;
    } else if (next >= colors.size()) {
        next = 0;
        g_warning("There are more Coloritems in the Toolbar then your Palette defines.\n"
                  "Hence, cycling through palette from the beginning.");
    }

    return result;
}

size_t Palette::getIndex() { return next; }

void Palette::reset() { next = 0; };

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

std::istream& operator>>(std::istream& str, Header& data) {
    std::string line;
    Header tmp;
    if (std::getline(str, line)) {
        std::istringstream iss{line};
        if (getline(iss, tmp.attribute, ':') && getline(iss, tmp.value, '\n')) {
            tmp.attribute = StringUtils::trim(tmp.attribute);
            tmp.value = StringUtils::trim(tmp.value);

            /* OK: All read operations worked */
            data.swap(tmp);  // C++03 as this answer was written a long time ago.
        } else {
            // One operation failed.
            // So set the state on the main stream
            // to indicate failure.
            str.setstate(std::ios::failbit);
        }
    }
    return str;
}

void Header::swap(Header& other)  // C++03 as this answer was written a long time ago.
{
    std::swap(attribute, other.attribute);
    std::swap(value, other.value);
}

// Default xournal
// 0 0 0 Black
// 0 128 0 Green
// 0 192 255 Light Blue
// 0 255 0 Light Green
// 51 51 204 Blue
// 128 128 128 Gray
// 255 0 0 Red
// 255 0 255 Magenta
// 255 128 0 Orange
// 255 255 0 Yellow
// 255 255 255 White
