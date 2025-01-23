#include "util/SaveNameUtils.h"

#include "include/util/SaveNameUtils.h"
#include "util/PathUtil.h"    // for clearExtensions


auto SaveNameUtils::parseFilenameFromWildcardString(const std::string& wildcardString, const fs::path& PdfPath,
                                                    const fs::path& FilePath) -> std::string {
    std::string saveString = wildcardString;
    size_t pos = saveString.find(DEFAULT_WILDCARD_START);

    // parse all wildcards until none are left
    while (pos != std::string::npos) {
        size_t wildcardStartLength = std::strlen(DEFAULT_WILDCARD_START);
        size_t endPos = saveString.find(DEFAULT_WILDCARD_END, pos + wildcardStartLength);
        if (endPos == std::string::npos) {
            break;
        }
        std::string parsedWildcard = parseWildcard(
                saveString.substr(pos + wildcardStartLength, endPos - pos - wildcardStartLength), PdfPath, FilePath);
        saveString.replace(pos, endPos + 1 - pos, parsedWildcard);
        pos += parsedWildcard.size();
        pos = saveString.find(DEFAULT_WILDCARD_START, pos);
    }

    return saveString;
}

auto SaveNameUtils::parseWildcard(const std::string& wildcard, const fs::path& PdfPath,
                                  const fs::path& FilePath) -> std::string {
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
        return wildcard == WILDCARD_DATE ? "%F" : "%X";
    }
    // not a valid wildcard
    return "";
}
