#include "util/PlaceholderString.h"

#include <exception>  // for exception

#include <glib.h>  // for g_error

#include "util/safe_casts.h"  // for as_unsigned

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

PlaceholderString::PlaceholderString(std::string text): text(std::move(text)) {}

auto PlaceholderString::formatPart(std::string format) const -> std::string {
    std::string formatDef;

    std::size_t comma = format.find(',');
    if (comma != std::string::npos) {
        formatDef = format.substr(comma + 1);
        format = format.substr(0, comma);
    }

    int index = 0;
    try {
        index = std::stoi(format);
    } catch (const std::exception& e) {
        g_error("Could not parse \"%s\" as int, error: %s", format.c_str(), e.what());
    }

    // Placeholder index starting at 1, vector at 0
    index--;

    if (index < 0 || index >= static_cast<int>(data.size())) {
        std::string notFound = "{";
        notFound += std::to_string(index + 1);
        notFound += "}";
        return notFound;
    }

    auto const& pe = data[as_unsigned(index)];

    return pe->format(formatDef);
}

void PlaceholderString::process() const {
    if (!processed.empty()) {
        return;
    }

    bool openBracket = false;
    bool closeBacket = false;
    std::string formatString;

    // Should work, also for UTF-8
    for (char c: text) {
        if (c == '{') {
            closeBacket = false;
            if (openBracket) {
                openBracket = false;
                processed += '{';
                continue;
            }
            openBracket = true;
            continue;
        }

        if (c == '}') {
            if (closeBacket) {
                processed += '}';
                closeBacket = false;
                continue;
            }

            closeBacket = true;
            if (openBracket) {
                processed += formatPart(formatString);
                openBracket = false;
                formatString = "";
            }
            continue;
        }

        if (openBracket) {
            formatString += c;
            continue;
        }


        closeBacket = false;
        processed += c;
    }
}

auto PlaceholderString::str() const -> std::string {
    process();
    return processed;
}

auto PlaceholderString::c_str() const -> const char* {
    process();
    return processed.c_str();
}

auto operator<<(std::ostream& os, PlaceholderString& ps) -> std::ostream& { return os << ps.str(); }
