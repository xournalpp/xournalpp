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

#include <fstream>  // std::ofstream

#include <execinfo.h>

#include "util/PathUtil.h"
#include "util/Stacktrace.h"

#include "config-dev.h"

static bool alreadyCrashed = false;

static void crashHandler(int sig);

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

    messages = backtrace_symbols(array, size);

    for (size_t i = 0; i < size; i++) {
        fp << FORMAT_STR("[bt]: ({1}) {2}") % i % messages[i];
        fp << "\n";
    }

    free(messages);

    fp << "\n\nTry to get a better stracktrace...\n";

    Stacktrace::printStracktrace(fp);

    if (fp) {
        fp.close();
    }

    emergencySave();

    exit(1);
}
