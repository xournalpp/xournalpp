#include "LatexGenerator.h"

#include <iomanip>  // for operator<<, setfill, setw
#include <regex>    // for smatch, sregex_iterator
#include <sstream>  // for ostringstream, basic_ost...

#include <glib.h>     // for GError, gchar, g_error_free
#include <poppler.h>  // for g_object_unref

#include "control/settings/LatexSettings.h"  // for LatexSettings
#include "util/PathUtil.h"                   // for getLongPath
#include "util/PlaceholderString.h"          // for PlaceholderString
#include "util/Util.h"                       // for Util
#include "util/i18n.h"                       // for FS, _F
#include "util/raii/GLibGuards.h"            // for GErrorGuard, GStrvGuard
#include "util/safe_casts.h"                 // for as_signed
#include "util/serdesstream.h"               // for serdes_stream


using namespace xoj::util;

LatexGenerator::LatexGenerator(const LatexSettings& settings): settings(settings) {}

auto LatexGenerator::templateSub(const std::string& input, const std::string& templ, const Color textColor)
        -> std::string {
    const static std::regex substRe("%%XPP_((TOOL_INPUT)|(TEXT_COLOR))%%");
    std::string output;
    output.reserve(templ.length());
    size_t templatePos = 0;
    for (std::sregex_iterator it(templ.begin(), templ.end(), substRe); it != std::sregex_iterator{}; it++) {
        std::smatch match = *it;
        std::string matchStr = match[1];
        std::string repl;
        // Performance can be optimized here by precomputing hashes
        if (matchStr == "TOOL_INPUT") {
            repl = input;
        } else if (matchStr == "TEXT_COLOR") {
            auto s = serdes_stream<std::ostringstream>();
            auto tmp_color = textColor;
            tmp_color.alpha = 0;
            s << std::hex << std::setfill('0') << std::setw(6) << std::right << tmp_color;
            repl = s.str();
        }
        output.append(templ, templatePos, as_unsigned(match.position()) - templatePos);
        output.append(repl);
        templatePos = as_unsigned(match.position() + match.length());
    }
    output.append(templ, templatePos);
    return output;
}

auto LatexGenerator::asyncRun(const fs::path& texDir, const std::string& texFileContents) -> Result {
    std::string cmd = this->settings.genCmd;
    GError* err = nullptr;
    GErrorGuard guard{&err};
    std::string texFilePathOSEncoding;
    try {
        texFilePathOSEncoding = (Util::getLongPath(texDir) / "tex.tex").string();
    } catch (fs::filesystem_error const& e) {
        GenError res{FS(_F("Failed to parse LaTeX generator path: {1}") % e.what())};
    }

    for (auto i = cmd.find(u8"{}"); i != std::string::npos; i = cmd.find(u8"{}", i + texFilePathOSEncoding.length())) {
        cmd.replace(i, 2, texFilePathOSEncoding);
    }
    // Todo (rolandlo): is this a todo?
    // Windows note: g_shell_parse_argv assumes POSIX paths, so Windows paths need to be escaped.
    gchar** argv = nullptr;
    GStrvGuard guard2{&argv};
    if (!g_shell_parse_argv(cmd.c_str(), nullptr, &argv, &err)) {
        return GenError{FS(_F("Failed to parse LaTeX generator command: {1}") % err->message)};
    }
    gchar* prog = argv[0];
    if (!prog || !(prog = g_find_program_in_path(prog))) {
        if (Util::isFlatpakInstallation()) {
            return GenError{
                    FS(_F("Failed to find LaTeX generator program in PATH: {1}\n\nSince installation is detected "
                          "within Flatpak, you need to install the Flatpak freedesktop Tex Live extension. For "
                          "example, by running:\n\n$ flatpak install flathub org.freedesktop.Sdk.Extension.texlive") %
                       argv[0])};
        } else {
            return GenError{FS(_F("Failed to find LaTeX generator program in PATH: {1}") % argv[0])};
        }
    }
    g_free(argv[0]);
    argv[0] = prog;

    if (!g_file_set_contents(texFilePathOSEncoding.c_str(), texFileContents.c_str(), as_signed(texFileContents.size()),
                             &err)) {
        return GenError({FS(_F("Could not save .tex file: {1}") % err->message)});
    }

    auto flags = static_cast<GSubprocessFlags>(G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE);
    GSubprocessLauncher* launcher = g_subprocess_launcher_new(flags);
    g_subprocess_launcher_set_cwd(launcher, texDir.u8string().c_str());
    auto* proc = g_subprocess_launcher_spawnv(launcher, argv, &err);

    std::string progName(prog);
    g_object_unref(launcher);

    if (proc) {
        return {proc};
    }
    return GenError(
            {FS(_F("Could not start {1}: {2} (exit code: {3})") % progName.c_str() % err->message % err->code)});
}
