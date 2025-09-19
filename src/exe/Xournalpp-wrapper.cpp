/*
 * Xournal++
 *
 * The main application
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#include <iostream>
#include <regex>
#include <sstream>
#include <string>

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "util/PathUtil.h"
#include "util/VersionInfo.h"
#include "util/XojMsgBox.h"
#include "util/glib_casts.h"
#include "util/gtk4_helper.h"
#include "util/raii/CStringWrapper.h"
#include "util/raii/GObjectSPtr.h"

#include "config.h"
#include "filesystem.h"

static constexpr const char* UI_RESOURCE = "/org/xournalpp/wrapper/ui/crashDialog.glade";

// See CrashHandler::emergencySave()
static constexpr const char* EMERGENCY_SAVE_MSG_REGEX = "Successfully saved document to \"(.*)\"";

static auto escape(const std::string& str) {
    return xoj::util::OwnedCString::assumeOwnership(g_uri_escape_string(str.c_str(), nullptr, true));
}

/// Create a URL which opens a crash report on github, pre-filled with the data we can get
static std::string makeCrashReportURL() {
    // See .github/ISSUE_TEMPLATE/crash_report.yml for the keywords (id)
    std::stringstream str;
    str.imbue(std::locale::classic());

    str << PROJECT_CRASHREPORT;
    str << "&os=" << escape(xoj::util::getOsInfo()).get();

#ifdef __linux__
    auto getEnvTruncated = [](const char* var) {
        if (auto* value = std::getenv(var); value) {
            auto d = std::string(value);
            if (d.length() > 20) {
                // No control over the environment variable: limit the length
                d.resize(20);
            }
            return d;
        } else {
            return std::string("unknown");
        }
    };
    str << "&desktop=" << escape(getEnvTruncated("DESKTOP_SESSION") + " " + getEnvTruncated("XDG_SESSION_TYPE")).get();
    str << "&displayserver=" << escape(xoj::util::getGdkBackend()).get();
#endif

    str << "&version=" << escape(xoj::util::getXournalppVersion()).get();
    str << "&gtk=" << gtk_major_version << "." << gtk_minor_version << "." << gtk_micro_version;

    return str.str();
}

static void activate(GtkApplication* app, std::string* str) {
    xoj::util::GObjectSPtr<GtkBuilder> builder(gtk_builder_new_from_resource(UI_RESOURCE), xoj::util::adopt);
    auto* window = GTK_WIDGET(gtk_builder_get_object(builder.get(), "crashDialog"));
    gtk_application_add_window(app, GTK_WINDOW(window));
    auto* view = GTK_TEXT_VIEW(gtk_builder_get_object(builder.get(), "textViewLog"));
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(view), str->data(), -1);

    auto* entry = GTK_ENTRY(gtk_builder_get_object(builder.get(), "entryPathEmergencySave"));
    std::smatch res;
    std::regex_search(*str, res, std::regex(EMERGENCY_SAVE_MSG_REGEX));
    if (res.empty()) {
        gtk_editable_set_text(GTK_EDITABLE(entry), "FAILED");
        gtk_widget_add_css_class(GTK_WIDGET(entry), "error");
    } else {
        gtk_editable_set_text(GTK_EDITABLE(entry), res[1].str().c_str());
    }

    g_signal_connect_object(gtk_builder_get_object(builder.get(), "closeBtn"), "clicked",
                            G_CALLBACK(+[](GtkButton* btn, gpointer w) { gtk_window_close(GTK_WINDOW(w)); }), window,
                            GConnectFlags(0));
    g_signal_connect_object(gtk_builder_get_object(builder.get(), "reportBtn"), "clicked",
                            G_CALLBACK(+[](GtkButton* btn, gpointer w) {
                                XojMsgBox::openURL(GTK_WINDOW(w), makeCrashReportURL().c_str());
                            }),
                            window, GConnectFlags(0));
    gtk_window_present(GTK_WINDOW(window));
}

auto main(int argc, char* argv[]) -> int {
    GError* err = nullptr;
    std::stringstream errorlog;
    errorlog.imbue(std::locale::classic());

    auto p = [&]() {
        static constexpr auto FLAGS = GSubprocessFlags(G_SUBPROCESS_FLAGS_STDOUT_PIPE
#ifdef _WIN32  // On Windows, STDERR_MERGE does not work. See https://gitlab.gnome.org/GNOME/glib/-/issues/3723
                                                       | G_SUBPROCESS_FLAGS_STDERR_PIPE
#else
                                                       | G_SUBPROCESS_FLAGS_STDERR_MERGE
#endif
        );

        std::vector<const char*> subargv;
        std::cout << Util::getExePath() << std::endl;

        const std::string path = (Util::getExePath() / "xournalpp").u8string();
        subargv.emplace_back(path.c_str());  // Data is owned by `path` - Do not delete it
        errorlog << "Executing \"" << path;

        for (int i = 1; i < argc; i++) {
            subargv.emplace_back(argv[i]);  // Data is owned by whatever called main()...
            errorlog << " " << argv[i];
        }
        subargv.emplace_back(nullptr);
        errorlog << "\"" << std::endl;

        xoj::util::GObjectSPtr<GSubprocessLauncher> launcher(g_subprocess_launcher_new(FLAGS), xoj::util::adopt);
        g_subprocess_launcher_set_environ(launcher.get(), nullptr);  // Copies the host's environment
        g_subprocess_launcher_setenv(launcher.get(), "G_MESSAGES_DEBUG", G_LOG_DOMAIN, false);  // Print xopp debug

        return xoj::util::GObjectSPtr<GSubprocess>(g_subprocess_launcher_spawnv(launcher.get(), subargv.data(), &err),
                                                   xoj::util::adopt);
    }();

    int exitstatus = -1;

    if (err) {
        printf("Failed to start\n");
        errorlog << "Failed to start: " << err->message << std::endl;
        g_error_free(err);
        err = nullptr;
    } else {
        xoj::util::OwnedCString stdoutBuffer;
#ifdef _WIN32  // On Windows, STDERR_MERGE does not work. See https://gitlab.gnome.org/GNOME/glib/-/issues/3723
        xoj::util::OwnedCString stderrBuffer;
#endif

        std::cout << "Xournal++ started with PID: " << g_subprocess_get_identifier(p.get()) << std::endl;
        errorlog << "Xournal++ started with PID: " << g_subprocess_get_identifier(p.get()) << std::endl;

        g_subprocess_communicate_utf8(p.get(), nullptr, nullptr, stdoutBuffer.contentReplacer(),
#ifdef _WIN32  // On Windows, STDERR_MERGE does not work. See https://gitlab.gnome.org/GNOME/glib/-/issues/3723
                                      stderrBuffer.contentReplacer(),
#else
                                      nullptr,
#endif
                                      &err);

        if (g_subprocess_get_if_exited(p.get())) {
            exitstatus = g_subprocess_get_exit_status(p.get());
            if (exitstatus == 0) [[likely]] {
                std::cout << "Execution completed normally" << std::endl;
                return 0;
            } else {
                std::cout << "Exited with error status: " << exitstatus << std::endl;
                errorlog << "Exited with error status: " << exitstatus << std::endl;
            }
        }
        if (g_subprocess_get_if_signaled(p.get())) {
            auto sig = g_subprocess_get_term_sig(p.get());
            std::cout << "Crashed with signal: " << sig << std::endl;
            errorlog << "Crashed with signal: " << sig << std::endl;
        }

        if (err) {
            errorlog << "    Error: " << err->message << std::endl;
            g_error_free(err);
            err = nullptr;
        }

        time_t lt = time(nullptr);
        errorlog << "    Date: " << ctime(&lt) << std::endl;

        errorlog << xoj::util::getVersionInfo();
        errorlog << "  (The GDK backend is probably printed out below)\n";

        errorlog << "\n*** Output: ***\n\n" << stdoutBuffer.get();

#ifdef _WIN32
        errorlog << "\n\n*** Cerr: ***\n\n" << stderrBuffer.get();
#endif
    }

    GtkApplication* app = gtk_application_new("com.github.xournalpp.xournalpp", G_APPLICATION_FLAGS_NONE);
    g_signal_connect_data(app, "activate", xoj::util::wrap_for_g_callback_v<activate>, new std::string(errorlog.str()),
                          xoj::util::closure_notify_cb<std::string>, GConnectFlags(0));

    g_application_run(G_APPLICATION(app), 0, nullptr);
    g_object_unref(app);

    return exitstatus;
}
