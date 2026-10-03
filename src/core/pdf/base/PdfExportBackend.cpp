#include "PdfExportBackend.h"

#include "util/i18n.h"

#include "config-features.h"

PdfExportBackend PdfExportBackend::fromString(const char* str) {
    return str != nullptr ? fromString(std::string_view(str)) : PdfExportBackend(PdfExportBackend::DEFAULT);
}

PdfExportBackend PdfExportBackend::fromString(std::string_view str) {
    if (str == "cairo") {
        return PdfExportBackend::CAIRO;
    }
#ifdef ENABLE_QPDF
    if (str == "qpdf") {
        return PdfExportBackend::QPDF;
    }
#endif
    if (str != DEFAULT_ID_STRING && !str.empty()) {
        g_warning("%s", (_F("Unknown pdf backend: {1}. Available backends are: {2}. Using default backend.") % str %
                         listAvailableBackends())
                                .c_str());
    }
    return PdfExportBackend::DEFAULT;
}

const char* PdfExportBackend::listAvailableBackends() {
    static const char* availablePdfExportBackends = "cairo"
#ifdef ENABLE_QPDF
                                                    " qpdf"
#endif
            ;
    return availablePdfExportBackends;
}

std::vector<std::pair<const char*, const char*>> PdfExportBackend::getPrettyNamesOfAvailableBackends() {
    std::vector<std::pair<const char*, const char*>> res;
    res.emplace_back(DEFAULT_ID_STRING, _("Default"));
    res.emplace_back("cairo", "Cairo");
#ifdef ENABLE_QPDF
    res.emplace_back("qpdf", "QPDF");
#endif
    return res;
}
