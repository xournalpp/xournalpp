#include "StrokeStyle.h"

#include "Stroke.h"

StrokeStyle::StrokeStyle() = default;

StrokeStyle::~StrokeStyle() = default;

const LineStyle dashLinePattern({6, 3}, 1.0);
const LineStyle dashDotLinePattern({6, 3, 0.5, 3}, 1.0);
const LineStyle dotLinePattern({0.5, 3}, 1.0);
const LineStyle heavyDownstrokePattern({}, 0.2);

#define PARSE_STYLE(name, def)      \
    if (strcmp(style, name) == 0) { \
        return def;                 \
    }

auto StrokeStyle::parseStyle(const char* style) -> LineStyle {
    PARSE_STYLE("dash", dashLinePattern);
    PARSE_STYLE("dashdot", dashDotLinePattern);
    PARSE_STYLE("dot", dotLinePattern);
    PARSE_STYLE("heavyDownstroke", heavyDownstrokePattern);

    if (strncmp("cust: ", style, 6) != 0) {
        return LineStyle();
    }

    // Todo(oddorb): support new options from here

    std::vector<double> dash;

    const char* widths = style + 6;
    while (*widths != 0) {
        char* tmpptr = nullptr;
        double val = g_ascii_strtod(widths, &tmpptr);
        if (tmpptr == widths) {
            break;
        }
        widths = tmpptr;
        dash.push_back(val);
    }

    if (dash.empty()) {
        return LineStyle();
    }

    auto* dashesArr = new double[dash.size()];
    for (int i = 0; i < static_cast<int>(dash.size()); i++) { dashesArr[i] = dash[i]; }

    LineStyle ls;
    ls.setDashes(dashesArr, static_cast<int>(dash.size()));
    delete[] dashesArr;

    return ls;
}

#define FORMAT_STYLE(name, def) \
    if (style == def) {         \
        return name;            \
    }

auto StrokeStyle::formatStyle(const LineStyle& style) -> std::string {
    FORMAT_STYLE("dash", dashLinePattern);
    FORMAT_STYLE("dashdot", dashDotLinePattern);
    FORMAT_STYLE("dot", dotLinePattern);
    FORMAT_STYLE("heavyDownstroke", heavyDownstrokePattern);

    // Todo(oddorb): support new options from here

    const double* dashes = nullptr;
    int dashCount = 0;
    if (style.getDashes(dashes, dashCount)) {
        std::string custom = "cust:";

        for (int i = 0; i < dashCount; i++) {
            custom += " ";
            char* str = g_strdup_printf("%0.2lf", dashes[i]);
            custom += str;
            g_free(str);
        }

        return custom;
    }

    // Should not be returned, in this case the attribute is not written
    return "plain";
}
