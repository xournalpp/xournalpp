/*
 * Xournal++
 *
 * This is the Linux / Unix Crash Handler implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <bitset>
#include <fstream>  // std::ofstream

#include <execinfo.h>
#include <gtk/gtk.h>

#include "util/PathUtil.h"
#include "util/Stacktrace.h"

#include "config-dev.h"
#include "config-git.h"
#include "config.h"

static bool alreadyCrashed = false;

static void crashHandler(int sig);

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
    sigset_t mask;

    sigemptyset(&mask);

#ifdef SIGSEGV
    signal(SIGSEGV, crashHandler);
    sigaddset(&mask, SIGSEGV);
#endif

#ifdef SIGFPE
    signal(SIGFPE, crashHandler);
    sigaddset(&mask, SIGFPE);
#endif

#ifdef SIGILL
    signal(SIGILL, crashHandler);
    sigaddset(&mask, SIGILL);
#endif

#ifdef SIGABRT
    signal(SIGABRT, crashHandler);
    sigaddset(&mask, SIGABRT);
#endif

    sigprocmask(SIG_UNBLOCK, &mask, 0);

    g_log_set_default_handler(GLogFunc(log_handler), getCrashHandlerLogBuffer());
}

/**
 * Print crash log to config directory
 */
static void crashHandler(int sig) {
    if (alreadyCrashed)  // crasehd again on emergency save
    {
        exit(2);
    }
    alreadyCrashed = true;

    g_warning("[Crash Handler] Crashed with signal %i", sig);

    time_t lt;
    void* array[100];
    char** messages;

    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 100);

    time_t curtime = time(0);
    char stime[128];
    strftime(stime, sizeof(stime), "%Y%m%d-%H%M%S", localtime(&curtime));
    auto const& errorlogPath = Util::getCacheSubfolder(ERRORLOG_DIR) / (std::string("errorlog.") + stime + ".log");
    std::ofstream fp(errorlogPath);
    if (fp) {
        g_warning("[Crash Handler] Wrote crash log to: %s", errorlogPath.c_str());
    }

    lt = time(nullptr);

    fp << FORMAT_STR("Date: {1}") % ctime(&lt);
    fp << FORMAT_STR("Error: signal {1}") % sig;
    fp << "\n";
    fp << "Xournal++ version " << PROJECT_VERSION << std::endl;

    if (auto const gitCommitId = std::string{GIT_COMMIT_ID}; !gitCommitId.empty()) {
        fp << "Git commit: " << gitCommitId << std::endl;
    }

    fp << "Gtk version " << gtk_get_major_version() << "." << gtk_get_minor_version() << "." << gtk_get_micro_version()
       << std::endl
       << std::endl;

    messages = backtrace_symbols(array, size);

    for (size_t i = 0; i < size; i++) {
        fp << FORMAT_STR("[bt]: ({1}) {2}") % i % messages[i];
        fp << "\n";
    }

    free(messages);

    fp << "\n\nTry to get a better stracktrace...\n";

    Stacktrace::printStracktrace(fp);

    fp << "\n\nExecution log:\n\n";
    fp << getCrashHandlerLogBuffer()->str();

    if (fp) {
        fp.close();
    }

    emergencySave();

    exit(1);
}
