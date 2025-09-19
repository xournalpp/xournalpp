/*
 * Xournal++
 *
 * PDF Document Export backend enum
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <string_view>
#include <utility>
#include <vector>

#include "util/i18n.h"

class ExportBackend {  // Must match the order in ui/exportSettings.glade:listPdfExportBackend
public:
    enum Value {
        DEFAULT,
        CAIRO,
        QPDF,
        // Keep last
        ENUM_END
    };

    ExportBackend() = default;
    constexpr ExportBackend(Value v): v(v) {}
    constexpr operator Value() const { return v; }
    explicit operator bool() const = delete;

    static constexpr auto DEFAULT_ID_STRING = "default";

    static ExportBackend fromString(std::string_view str);
    static ExportBackend fromString(const char* str);
    static const char* listAvailableBackends();
    static std::vector<std::pair<const char*, const char*>> getPrettyNamesOfAvailableBackends();

private:
    Value v;
};
