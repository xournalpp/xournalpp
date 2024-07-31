#include "util/SaveNameUtils.h"

#include <string>

#include "util/PathUtil.h"  // for clearExtensions


auto SaveNameUtils::parseFilenameFromWildcardString(const std::string& wildcardString, const fs::path& defaultFilePath) -> std::string {
    std::string saveString = wildcardString;
    size_t pos = saveString.find(DEFAULT_WILDCARD_START);

    // parse all wildcards until none are left
    while (pos != std::string::npos) {
        size_t wildcardStartLength = std::string_view(DEFAULT_WILDCARD_START).length();
        size_t endPos = saveString.find(DEFAULT_WILDCARD_END, pos + wildcardStartLength);
        if (endPos == std::string::npos) {
            break;
        }
        std::string parsedWildcard = parseWildcard(saveString.substr(pos + wildcardStartLength, endPos - pos - wildcardStartLength), defaultFilePath);
        saveString.replace(pos, endPos + 1 - pos, parsedWildcard);
        pos += parsedWildcard.size();
        pos = saveString.find(DEFAULT_WILDCARD_START, pos);
    }

    return saveString;
}

auto SaveNameUtils::parseWildcard(const std::string& wildcard, const fs::path& defaultFilePath) -> std::string {
    if (wildcard == WILDCARD_NAME) {
        fs::path path = defaultFilePath;
        Util::clearExtensions(path, ".pdf");
        auto u8str = path.u8string();
        return {u8str.begin(), u8str.end()};
    }
    if (wildcard == WILDCARD_DATE || wildcard == WILDCARD_TIME) {
        // Backwards compatibility: redirect to std::chrono placeholders
        return wildcard == WILDCARD_DATE ? "%F" : "%X";
    }
    // not a valid wildcard
    return "";
}
