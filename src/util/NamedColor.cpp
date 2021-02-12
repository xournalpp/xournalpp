#include "NamedColor.h"

#include <sstream>

#include <util/StringUtils.h>

std::istream& operator>>(std::istream& str, NamedColor& data) {
    std::string line;
    NamedColor tmp;
    if (std::getline(str, line)) {
        std::istringstream iss{line};
        if (iss >> tmp.color.red >> tmp.color.blue >> tmp.color.green && std::getline(iss, tmp.name)) {
            tmp.name = StringUtils::trim(tmp.name);

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
void NamedColor::swap(NamedColor& other)  // C++03 as this answer was written a long time ago.
{
    std::swap(color.red, other.color.red);
    std::swap(color.green, other.color.green);
    std::swap(color.blue, other.color.blue);
    std::swap(color.alpha, other.color.alpha);
    std::swap(name, other.name);
}