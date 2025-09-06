#include "util/SaveNameUtils.h"

#include "util/PathUtil.h"  // for clearExtensions


auto SaveNameUtils::parseFilenameFromWildcardString(std::u8string_view wildcardString, const fs::path& PdfPath,
                                                    const fs::path& FilePath) -> std::u8string {
    std::u8string saveString = wildcardString.data();
    size_t pos = saveString.find(DEFAULT_WILDCARD_START);

    // parse all wildcards until none are left
    while (pos != std::string::npos) {
        size_t wildcardStartLength = std::u8string_view(DEFAULT_WILDCARD_START).length();
        size_t endPos = saveString.find(DEFAULT_WILDCARD_END, pos + wildcardStartLength);
        if (endPos == std::u8string::npos) {
            break;
        }
        std::u8string parsedWildcard = parseWildcard(
                saveString.substr(pos + wildcardStartLength, endPos - pos - wildcardStartLength), PdfPath, FilePath);
        saveString.replace(pos, endPos + 1 - pos, parsedWildcard);
        pos += parsedWildcard.size();
        pos = saveString.find(DEFAULT_WILDCARD_START, pos);
    }

    return saveString;
}

auto SaveNameUtils::parseWildcard(std::u8string_view wildcard, const fs::path& PdfPath, const fs::path& FilePath)
        -> std::u8string {
    if (wildcard == WILDCARD_PDF_NAME) {
        fs::path path = PdfPath;
        Util::clearExtensions(path, ".pdf");
        return path.u8string();
    }

    if (wildcard == WILDCARD_FILE_NAME) {
        fs::path path = FilePath;
        Util::clearExtensions(path, "");
        return path.u8string();
    }

    if (wildcard == WILDCARD_DATE || wildcard == WILDCARD_TIME) {
        // Backwards compatibility: redirect to std::chrono placeholders
        return wildcard == WILDCARD_DATE ? u8"%F" : u8"%X";
    }
    // not a valid wildcard
    return u8"";
}
