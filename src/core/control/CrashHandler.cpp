#include "CrashHandler.h"

#include <array>   // for std::array
#include <atomic>  // for std::atomic
#include <bitset>
#include <csignal>  // for std::signal
#include <fstream>  // for std::ofstream
#include <sstream>
#include <string>       // for std::string
#include <string_view>  // for std::string_view

#include <fbbe/stacktrace.h>  // for fbbe::stacktrace
#include <glib.h>             // for g_warning, g_error
#include <gtk/gtk.h>

#include "control/xojfile/SaveHandler.h"  // for SaveHandler
#include "util/PathUtil.h"
#include "util/PathUtil.h"    // for getConfigFile
#include "util/Stacktrace.h"  // for Stacktrace::printStracktrace
#include "util/i18n.h"        // for FC, _F, _

#include "config-dev.h"
#include "config-git.h"
#include "config.h"
#include "filesystem.h"  // for path


static std::atomic<Document*> document = nullptr;
static std::atomic<bool> alreadyCrashed = false;
static std::stringstream logBuffer;

void setEmergencyDocument(Document* doc) { document = doc; }
static std::stringstream* getCrashHandlerLogBuffer() { return &logBuffer; }


/**
 * Print crash log to config directory
 */
static void crashHandler(int sig) {
    if (alreadyCrashed) {  // crasehd again on emergency save
        exit(2);
    }
    alreadyCrashed = true;

    g_warning("[Crash Handler] Crashed with signal %i", sig);

    auto curtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::array<char, 128> stime;
    size_t size = 0;
    {
        static constexpr std::array<char, 10> error_log_name{"errorlog."};
        std::memcpy(stime.data(), error_log_name.data(), error_log_name.size() - 1);
        assert(stime[8] == '.');
        size = strftime(&stime[9], stime.size() - error_log_name.size(), "%Y%m%d-%H%M%S", localtime(&curtime));
        std::memcpy(stime.data() + size + 9, ".log", 4);
    }
    auto const& errorlogPath = Util::getCacheSubfolder(ERRORLOG_DIR) / std::string_view(stime.data(), size + 9 + 4);
    std::ofstream fp(errorlogPath);
    if (fp) {}

    fp << FORMAT_STR("Date: {1}") % ctime(&curtime);
    fp << FORMAT_STR("Error: signal {1}") % sig;
    fp << "\n";
    fp << "Xournal++ version " << PROJECT_VERSION << "\n";

    if (auto const gitCommitId = std::string_view{GIT_COMMIT_ID}; !gitCommitId.empty()) {
        fp << "Git commit: " << gitCommitId << "\n";
    }

    fp << "Gtk version " << gtk_get_major_version() << "." << gtk_get_minor_version() << "." << gtk_get_micro_version()
       << "\n"
       << std::endl;

    Stacktrace::printStracktrace(std::cerr);
    Stacktrace::printStracktrace(fp);

    fp << "\n\nExecution log:\n\n";
    fp << getCrashHandlerLogBuffer()->str();

    if (fp) {
        g_warning("[Crash Handler] Wrote crash log to: %s", errorlogPath.c_str());
        fp.close();
    }

    emergencySave();

    exit(1);
}

constexpr GLogLevelFlags RECORDED_LOG_LEVELS = GLogLevelFlags(0xff);  //< Record all messages

static void log_handler(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message,
                        std::stringstream* logBuffer) {
    if ((log_level & RECORDED_LOG_LEVELS) != 0) {
        if (log_level & G_LOG_FLAG_FATAL) {
            *logBuffer << "FATAL ";
        }
        *logBuffer << std::setw(8) << std::left;
        switch (log_level & G_LOG_LEVEL_MASK) {
            case G_LOG_LEVEL_ERROR:
                *logBuffer << "ERROR";
                break;
            case G_LOG_LEVEL_CRITICAL:
                *logBuffer << "CRITICAL";
                break;
            case G_LOG_LEVEL_WARNING:
                *logBuffer << "WARNING";
                break;
            case G_LOG_LEVEL_MESSAGE:
                *logBuffer << "MESSAGE";
                break;
            case G_LOG_LEVEL_INFO:
                *logBuffer << "INFO";
                break;
            case G_LOG_LEVEL_DEBUG:
                *logBuffer << "DEBUG";
                break;
            default:
                *logBuffer << std::bitset<8>(static_cast<unsigned int>(log_level));
        }
        *logBuffer << ": " << (log_domain ? log_domain : "--") << " :: " << (message ? message : "No message")
                   << std::endl;
    }

    // Forward to stderr/stdout
    g_log_default_handler(log_domain, log_level, message, nullptr);
}

void installCrashHandlers() {
    std::signal(SIGTERM, crashHandler);
    std::signal(SIGINT, crashHandler);
    std::signal(SIGSEGV, crashHandler);
    std::signal(SIGFPE, crashHandler);
    std::signal(SIGILL, crashHandler);
    std::signal(SIGABRT, crashHandler);
    g_log_set_default_handler(GLogFunc(log_handler), getCrashHandlerLogBuffer());
}

void emergencySave() {
    if (document == nullptr) {
        return;
    }

    g_warning(_("Trying to emergency save the current open document…"));

    auto const& filepath = Util::getConfigFile("emergencysave.xopp");

    SaveHandler handler;
    handler.prepareSave(document);
    handler.saveTo(filepath);

    if (!handler.getErrorMessage().empty()) {
        g_error("%s", FC(_F("Error: {1}") % handler.getErrorMessage()));
    } else {
        g_warning("%s", FC(_F("Successfully saved document to \"{1}\"") % filepath.string()));
    }
}
