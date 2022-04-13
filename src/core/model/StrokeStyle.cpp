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

    std::vector<double> dashes;

    const char* curr = style + 6;
    while (*curr != '\0') {
        char* endptr = nullptr;
        double val = g_ascii_strtod(curr, &endptr);
        if (endptr == curr) {
            break;
        }
        curr = endptr;
        dashes.push_back(val);
    }

    if (dashes.empty()) {
        return LineStyle();
    }

    double heavy_downstroke_ratio = 1.0;

    while (isspace(*curr)) { curr++; }
    if (*curr != '\0') {
        if (strncmp(curr, "hd_ratio(", 9) == 0) {
            curr += 9;
            char* endptr = nullptr;
            double val = g_ascii_strtod(curr, &endptr);
            if (endptr != curr) {
                heavy_downstroke_ratio = val;
            }
            curr = endptr;
        }
    }

    LineStyle ls(dashes, heavy_downstroke_ratio);

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

        if (style.hasHeavyDownstroke()) {
            custom += "hd_ratio(";
            char* str = g_strdup_printf("%0.2lf", style.getHeavyDownstrokeRatio());
            custom += str;
            custom += ")";
            g_free(str);
        }

        return custom;
    }

    // Should not be returned, in this case the attribute is not written
    return "plain";
}
