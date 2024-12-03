#include "util/SaveNameUtils.h"

#include "util/PathUtil.h"    // for clearExtensions


auto SaveNameUtils::parseFilenameFromWildcardString(std::string_view wildcardString,
                                                    const fs::path& defaultFilePath) -> std::string {
    std::string saveString{wildcardString};
    size_t pos = saveString.find(DEFAULT_WILDCARD_START);

    // parse all wildcards until none are left
    while (pos != std::string::npos) {
        size_t wildcardStartLength = std::strlen(DEFAULT_WILDCARD_START);
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

auto SaveNameUtils::parseWildcard(std::string_view wildcard, const fs::path& defaultFilePath) -> std::string {
    if (wildcard == WILDCARD_NAME) {
        fs::path path = defaultFilePath;
        Util::clearExtensions(path, ".pdf");
        return path.u8string();
    }
    if (wildcard == WILDCARD_DATE || wildcard == WILDCARD_TIME) {
        // Backwards compatibility: redirect to std::chrono placeholders
        return wildcard == WILDCARD_DATE ? "%F" : "%X";
    }
    // not a valid wildcard
    return "";
}
