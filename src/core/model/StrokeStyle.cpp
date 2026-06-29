#include "StrokeStyle.h"

#include <cstring>   // for memcmp, strcmp, strncmp
#include <iomanip>   // for setprecision
#include <iterator>  // for ostream_iterator
#include <map>       // for map
#include <sstream>   // for istringstream
#include <vector>    // for vector

#include "model/LineStyle.h"  // for LineStyle
#include "util/serdesstream.h"

namespace {

constexpr auto CUSTOM_KEY = "cust: ";
constexpr auto SCALED_CUSTOM_KEY = "scaled_cust: ";

const std::map<std::string, std::vector<double>> predefinedPatterns = {
        {"dash", {6, 3}},        {"dashdot", {6, 3, 0.5, 3}},        {"dot", {0.5, 3}},
        {"scaled_dash", {6, 3}}, {"scaled_dashdot", {6, 3, 0.5, 3}}, {"scaled_dot", {0.5, 3}}};

auto formatStyle(const std::vector<double>& dashes, bool scaleDashes) -> std::string {

    // Check if dashes match named predefined dashes.
    for (auto& pair: predefinedPatterns) {
        if (pair.second == dashes && scaleDashes == pair.first.starts_with("scaled")) {
            return pair.first;
        }
    }

    // Else generate custom dashes string
    auto custom = serdes_stream<std::ostringstream>();
    custom << std::setprecision(2) << std::fixed;
    custom << (scaleDashes ? SCALED_CUSTOM_KEY : CUSTOM_KEY);
    std::copy(dashes.begin(), dashes.end(), std::ostream_iterator<double>(custom, " "));

    // Return dashes string with traling space removed.
    return custom.str().substr(0, custom.str().length() - 1);
}

}  // namespace

auto StrokeStyle::parseStyle(const std::string& style) -> LineStyle {
    auto it = predefinedPatterns.find(style);
    bool scaleDashes = style.starts_with("scaled");

    if (it != predefinedPatterns.end()) {
        LineStyle ls;
        std::vector<double> dashes = it->second;
        ls.setDashes(std::move(dashes));

        if (scaleDashes) {
            ls.setScaleDashes();
        }

        return ls;
    }

    const char* customKey = scaleDashes ? SCALED_CUSTOM_KEY : CUSTOM_KEY;
    if (style.substr(0, strlen(customKey)) != customKey) {
        return LineStyle();
    }

    auto dashStream = serdes_stream<std::stringstream>(style);
    std::vector<double> dashes;

    dashStream.seekg(strlen(customKey));
    for (double value; dashStream >> value;) {
        dashes.push_back(value);
    }

    if (dashes.empty()) {
        return LineStyle();
    }

    LineStyle ls;
    ls.setDashes(std::move(dashes));

    if (scaleDashes) {
        ls.setScaleDashes();
    }

    return ls;
}

auto StrokeStyle::formatStyle(const LineStyle& style) -> std::string {
    const auto& dashes = style.getDashes();
    if (!dashes.empty()) {
        return ::formatStyle(dashes, style.scaleDashes());
    }

    // Should not be returned, in this case the attribute is not written
    return "plain";
}
