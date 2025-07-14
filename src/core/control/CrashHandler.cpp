#include "CrashHandler.h"

#include <atomic>
#include <csignal>
#include <iostream>
#include <string>

#include "control/xojfile/SaveHandler.h"  // for SaveHandler
#include "util/PathUtil.h"
#include "util/Stacktrace.h"
#include "util/VersionInfo.h"

#include "filesystem.h"  // for path

static std::atomic<const Document*> document = nullptr;
static std::atomic<int> alreadyCrashed = 0;
static std::atomic<bool> sigint = false;

extern "C" void forceClose(int sig) {
    g_warning("Force close requested with signal %i", sig);
    emergencySave();
    exit(1);
}

extern "C" void gracefullyClose(int sig) {
    // Do not add more code here, because this function is called from a signal handler
    // Therefore only async-signal-safe functions are allowed here
    // To prevent corruption of the document, we only set a flag here and do the actual work in the main (eventloop)
    // thread. See Control::Control() where the flag is handled.

    // The signal handlers are defaulted after a signal is caught, so we have to set them again
    std::signal(SIGINT, gracefullyClose);
    if (sigint.exchange(true)) {
        g_warning("Ignored second gracefully close request with signal %i, force closing...", sig);
        forceClose(sig);
    }
    g_warning("Gracefully close requested with signal %i", sig);
}

auto interrupted() -> bool { return sigint.exchange(false); }

void setEmergencyDocument(const Document* doc) { document = doc; }

void emergencySave() {
    if (document == nullptr) {
        return;
    }

    std::cerr << "Trying to emergency save the current open document..." << std::endl;

    auto const& filepath = Util::getConfigFile("emergencysave.xopp");

    SaveHandler handler;
    handler.prepareSave(document, filepath);
    handler.saveTo(filepath);

    if (!handler.getErrorMessage().empty()) {
        std::cerr << "Error: " << handler.getErrorMessage() << std::endl;
    } else {
        std::cerr << "Successfully saved document to \"" << filepath.u8string() << "\"" << std::endl;
    }
}


/// Print a backtrace and try to make an emergency save
extern "C" void crashHandler(int sig) {
    int crash = ++alreadyCrashed;

    std::cerr << "\n\n\n*************************************************************************\n\n";
    std::cerr << "[Crash Handler] Crashed " << crash << " time(s) with signal " << sig << std::endl;
    if (crash == 1) {  // Avoid a loop in case we crash again on emergencySave()
        std::cerr << xoj::util::getVersionInfo() << std::endl;

        std::cerr << "\nTry to get a stacktrace...\n";

        Stacktrace::printStacktrace(std::cerr);
        std::cerr << std::endl;

        emergencySave();
    }

#ifdef __unix__
    // Forward the signal for the system's default handling - we may get a coredump
    std::signal(sig, SIG_DFL);
    kill(getpid(), sig);
#endif

    exit(crash);
}

void installCrashHandlers() {
    std::signal(SIGTERM, forceClose);
    std::signal(SIGINT, gracefullyClose);
    std::signal(SIGSEGV, crashHandler);
    std::signal(SIGFPE, crashHandler);
    std::signal(SIGILL, crashHandler);
    std::signal(SIGABRT, crashHandler);
#ifdef SIGTRAP
    std::signal(SIGTRAP, crashHandler);
#endif
}
