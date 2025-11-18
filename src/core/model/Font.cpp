#include "Font.h"

#include <cstdlib>  // for atof
#include <regex>    // for match_results, rege...
#include <sstream>  // for basic_ostream, oper...
#include <utility>  // for move

#include "util/serdesstream.h"
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

XojFont::XojFont(std::string name, double size) {
    setName(std::move(name));
    setSize(size);
}

auto XojFont::getName() const -> const std::string& { return this->name; }

void XojFont::setName(std::string name) { this->name = std::move(name); }

auto XojFont::getSize() const -> double { return size; }

void XojFont::setSize(double size) { this->size = size; }

XojFont::XojFont(const char* description) {
    // See https://stackoverflow.com/questions/44949784/c-regex-which-group-matched for
    // a good overview of regular expressions in C++.
    std::regex pangoFontDescriptionRegex{"^(.*) (\\d+[.]?\\d*)$"};

    std::match_results<const char*> results;
    std::regex_search(description, results, pangoFontDescriptionRegex);

    if (results.size() > 1) {
        this->name = results[1].str();
    } else {
        this->name = "";
    }

    if (results.size() > 2) {
        this->size = std::stod(results[2].str());
    } else {
        this->size = 0;
    }
}

XojFont& XojFont::operator=(const std::string& description) { return *this = XojFont(description.c_str()); }

auto XojFont::asString() const -> std::string {
    auto result = serdes_stream<std::stringstream>();
    result << getName() << " " << getSize();

    return result.str();
}

void XojFont::serialize(ObjectOutputStream& out) const {
    out.writeObject("XojFont");

    out.writeString(this->name);
    out.writeDouble(this->size);

    out.endObject();
}

void XojFont::readSerialized(ObjectInputStream& in) {
    in.readObject("XojFont");

    this->name = in.readString();
    this->size = in.readDouble();

    in.endObject();
}
