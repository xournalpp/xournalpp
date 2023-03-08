#include "StrokeStyle.h"

#include <cstring>  // for memcmp, strcmp, strncmp
#include <iomanip>  // for setprecision
#include <iterator> // for ostream_iterator
#include <map>      // for map
#include <sstream>  // for istringstream
#include <vector>   // for vector

#include "model/LineStyle.h"  // for LineStyle

StrokeStyle::StrokeStyle() = default;

StrokeStyle::~StrokeStyle() = default;

namespace {

constexpr auto CUST_KEY = "cust: ";

const std::map<std::string, std::vector<double>> predefinedPatterns = {
    { "dash", {6, 3} }, 
    { "dashdot", {6, 3, 0.5, 3} },
    { "dot", {0.5, 3}}
};

auto formatStyle(const double* dashes, int count) -> std::string {

    // Check if dashes match named predefined dashes.
    std::vector<double> input(dashes, dashes + count);
    for (auto &pair: predefinedPatterns ) {
        if (pair.second == input) {
            return pair.first;
        }
    }

    // Else generate custom dashes string
    std::ostringstream custom;
    custom << std::setprecision(2) << std::fixed;
    custom << CUST_KEY;
    std::copy(input.begin(), input.end(),std::ostream_iterator<double>(custom," "));

    // Return dashes string with traling space removed.
    return custom.str().substr(0, custom.str().length() - 1);
}

}

auto StrokeStyle::parseStyle(const std::string &style) -> LineStyle {
    auto it = predefinedPatterns.find(style);
    if (it != predefinedPatterns.end()) {
        LineStyle ls;
        ls.setDashes(it->second.data(), static_cast<int>(it->second.size()));
        return ls;
    }

    if (style.substr(0, strlen(CUST_KEY)) != CUST_KEY) {
        return LineStyle();
    }

    std::stringstream dashStream(style);
    std::vector<double> dash;

    dashStream.seekg(strlen(CUST_KEY));
    for (double value; dashStream >> value;)
        dash.push_back(value);

    if (dash.empty()) {
        return LineStyle();
    }

    LineStyle ls;
    ls.setDashes(dash.data(), static_cast<int>(dash.size()));
    return ls;
}

auto StrokeStyle::formatStyle(const LineStyle& style) -> std::string {
    const double* dashes = nullptr;
    int dashCount = 0;
    if (style.getDashes(dashes, dashCount)) {
        return ::formatStyle(dashes, dashCount);
    }

    // Should not be returned, in this case the attribute is not written
    return "plain";
}
