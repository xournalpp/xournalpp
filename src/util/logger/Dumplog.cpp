#include "util/logger/Dumplog.h"

#include <array>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <locale>
#include <vector>

#include <glib.h>

#include "util/PathUtil.h"

#include "config-dev.h"
#include "config.h"
#include "filesystem.h"

namespace xoj::util::log {

auto printBugreportNotice(std::ostream& stream) -> std::ostream& {
    return stream << "Please report this issue at\n  " << PROJECT_BUGREPORT;
}

namespace {

constexpr std::array dumplogFileNames = {
        "errorlog",  // Dumplog::ERRORLOG
        "deadlog"    // Dumplog::DEADLOG
};

}  // namespace

class DumplogPrivate {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    DumplogPrivate(Dumplog::Type type, const char* msgPrefix, Dumplog::PredefinedHeaderFlags header):
            time_(::time(nullptr)), msgPrefix_(msgPrefix), headerFlags_(header) {
                
        std::array<char, 20> filenameTime{};
        strftime(filenameTime.data(), filenameTime.size(), "%Y%m%d-%H%M%S", localtime(&time_));
        errorlogPath_ /= std::string(dumplogFileNames.at(type)).append(".").append(filenameTime.data()).append(".log");

        stream_.imbue(std::locale::classic());
    }

    ~DumplogPrivate() {
        std::ofstream f(errorlogPath_);

        if (headerFlags_ & Dumplog::HEADER_TIME) {
            f << "Date: " << ctime(&time_);
        }
        for (auto& l: headerLines_) { f << l << '\n'; }

        f << '\n' << stream_.str() << std::flush;

        if (f) {
            g_warning("%sWrote log to: %s", msgPrefix_, errorlogPath_.u8string().c_str());
        } else {
            g_warning("%sError writing log file.", msgPrefix_);
        }
    }

    auto stream() -> std::ostringstream& { return stream_; }

    void addHeaderLine(std::string l) { headerLines_.push_back(std::move(l)); }

private:
    std::ostringstream stream_;
    fs::path errorlogPath_{Util::getCacheSubfolder(ERRORLOG_DIR)};
    time_t time_;
    const char* msgPrefix_;
    Dumplog::PredefinedHeaderFlags headerFlags_;
    std::vector<std::string> headerLines_;
};

Dumplog::Dumplog(Type type, const char* msgPrefix, PredefinedHeaderFlags header):
        impl_(new DumplogPrivate(type, msgPrefix, header)) {}
Dumplog::Dumplog(Dumplog&& d): impl_(std::move(d.impl_)) {}
Dumplog::~Dumplog() {}

auto Dumplog::stream() -> std::ostream& { return impl_->stream(); }
auto Dumplog::str() const -> std::string { return impl_->stream().str(); }

void Dumplog::addHeaderLine(std::string l) { impl_->addHeaderLine(std::move(l)); }

}  // namespace xoj::util::log
