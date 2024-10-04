#include "ColorPalette.h"

#include <fstream>    // for basic_ifstream, basic_ofstream
#include <sstream>    // for operator<<, basic_ostream::operator<<
#include <stdexcept>  // for invalid_argument
#include <utility>    // for move

#include <glib.h>  // for g_warning, g_error

#include "util/StringUtils.h"  // for StringUtils
#include "util/Util.h"
#include "util/XojMsgBox.h"
#include "util/i18n.h"          // for FORMAT_STR, FS, _
#include "util/serdesstream.h"  // for serdes_stream


Palette::Palette(fs::path path): filepath{std::move(path)}, namedColors{}, header{} {}

void Palette::load() {
    if (!fs::exists(this->filepath))
        throw std::invalid_argument(
                FS(FORMAT_STR("The palette file {1} does not exist.") % this->filepath.filename().u8string()));
    header.clear();
    namedColors.clear();


    auto gplFile = serdes_stream<std::ifstream>(filepath);
    gplFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::string line;
    getline(gplFile, line);
    int lineNumber{1};
    // parse standard header line
    parseFirstGimpPaletteLine(line);
    // attempt parsing line by line as either header, color, or fallback
    while (!gplFile.eof() && gplFile.peek() != EOF && getline(gplFile, line)) {
        lineNumber++;
        parseCommentLine(line) || parseHeaderLine(line) || parseColorLine(line) || parseEmptyLine(line) ||
                parseLineFallback(lineNumber);
    }
    if (namedColors.size() < 1) {
        throw std::invalid_argument("Your Palettefile has no parsable color. It needs at least one!");
    }
}

auto Palette::load_default() -> void {
    std::stringstream defaultFile{default_palette()};
    std::string line;

    getline(defaultFile, line);
    if (!parseFirstGimpPaletteLine(line))
        g_error("The default file was mallformed. This should never happen!");
    while (getline(defaultFile, line)) {
        parseHeaderLine(line) || parseColorLine(line);
    }
}

auto Palette::getHeader(const std::string& attr) const -> std::string {
    if (header.find(attr) == header.end()) {
        return std::string{};
    }
    return header.at(attr);
}

auto Palette::parseFirstGimpPaletteLine(const std::string& line) const -> bool {
    if (StringUtils::trim(line) != "GIMP Palette") {
        throw std::invalid_argument(".gpl file needs to start with \"GIMP Palette\" in the "
                                    "first line");
    }
    return true;
}

auto Palette::parseHeaderLine(const std::string& line) -> bool {
    Header headerEntry;
    std::istringstream iss{line};
    if (iss >> headerEntry) {
        header[headerEntry.getAttribute()] = headerEntry.getValue();
        return true;
    }
    return false;
}

auto Palette::parseColorLine(const std::string& line) -> bool {
    NamedColor color{namedColors.size()};
    std::istringstream iss{line};
    if (iss >> color) {
        namedColors.emplace_back(std::move(color));
        return true;
    }
    return false;
}

auto Palette::parseCommentLine(const std::string& line) const -> bool { return line.front() == '#'; }

auto Palette::parseEmptyLine(const std::string& line) const -> bool { return line.empty(); }

auto Palette::parseLineFallback(int lineNumber) const -> const bool {
    throw std::invalid_argument(FS(FORMAT_STR("The line {1} is malformed.") % lineNumber));
}

auto Palette::getColorAt(size_t i) const -> NamedColor const& {
    if (i >= namedColors.size()) {
        i = i % namedColors.size();
        g_warning("There are more Coloritems in the Toolbar than your Palette defines.\n"
                  "Hence, cycling through palette from the beginning.");
    }

    return namedColors.at(i);
}

auto Palette::size() const -> size_t { return namedColors.size(); }


auto Palette::default_palette() -> const std::string {
    auto d = serdes_stream<std::stringstream>();
    d << "GIMP Palette\n"
      << "Name: Xournal Palette\n"
      << "#\n"
      << 0 << " " << 0 << " " << 0 << " " << NC_("Color", "Black") << "\n"
      << 0 << " " << 128 << " " << 0 << " " << NC_("Color", "Green") << "\n"
      << 0 << " " << 192 << " " << 255 << " " << NC_("Color", "Light Blue") << "\n"
      << 0 << " " << 255 << " " << 0 << " " << NC_("Color", "Light Green") << "\n"
      << 51 << " " << 51 << " " << 204 << " " << NC_("Color", "Blue") << "\n"
      << 128 << " " << 128 << " " << 128 << " " << NC_("Color", "Gray") << "\n"
      << 255 << " " << 0 << " " << 0 << " " << NC_("Color", "Red") << "\n"
      << 255 << " " << 0 << " " << 255 << " " << NC_("Color", "Magenta") << "\n"
      << 255 << " " << 128 << " " << 0 << " " << NC_("Color", "Orange") << "\n"
      << 255 << " " << 255 << " " << 0 << " " << NC_("Color", "Yellow") << "\n"
      << 255 << " " << 255 << " " << 255 << " " << NC_("Color", "White") << std::endl;
    return d.str();
}

void Palette::create_default(fs::path filepath) {
    auto myfile = serdes_stream<std::ofstream>(filepath);
    myfile << default_palette();
}

auto Header::getAttribute() const -> std::string { return this->attribute; };
auto Header::getValue() const -> std::string { return this->value; };
auto Palette::getFilePath() const -> fs::path const& { return this->filepath; };

auto operator>>(std::istream& str, Header& header) -> std::istream& {
    /*
     * inspired by
     * https://codereview.stackexchange.com/questions/38879/parsing-text-file-in-c
     */
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
            /*
             * One operation failed.
             * So set the state on the main stream
             * to indicate failure.
             */
            str.setstate(std::ios::failbit);
        }
    }
    return str;
}

auto Palette::parseErrorDialog(const std::exception& e) const -> void {
    std::stringstream msg_stream{};
    msg_stream << "There has been a problem parsing the color palette file at " << filepath.c_str() << "\n\n";
    msg_stream << "What happened:\n" << e.what() << std::endl;
    msg_stream << "What to do:\n";
    msg_stream << "Please fix your palette file, or rename it so xournalpp creates a new default palette file "
                  "for you. This file can then be used as a template.\n";
    msg_stream << "Until this is fixed, the application will use the default color palette.";

    // Call later, to make sure the main window has been set up, so the popup is displayed in front of it (and modal)
    Util::execWhenIdle([msg = msg_stream.str()]() { XojMsgBox::showErrorToUser(nullptr, msg); });
}
