#include "LatexGenerator.h"

#include <iomanip>
#include <iterator>
#include <regex>
#include <sstream>

#include <glib.h>

#include "i18n.h"

LatexGenerator::LatexGenerator(const LatexSettings& settings): settings(settings) {}

auto LatexGenerator::templateSub(const std::string& input, const std::string& templ, const Color textColor)
        -> std::string {
    const static std::regex substRe("%%XPP_((TOOL_INPUT)|(TEXT_COLOR))%%");
    std::string output;
    output.reserve(templ.length());
    int templatePos = 0;
    for (std::sregex_iterator it(templ.begin(), templ.end(), substRe); it != std::sregex_iterator{}; it++) {
        std::smatch match = *it;
        std::string matchStr = match[1];
        std::string repl;
        // Performance can be optimized here by precomputing hashes
        if (matchStr == "TOOL_INPUT") {
            repl = input;
        } else if (matchStr == "TEXT_COLOR") {
            std::ostringstream s;
            s.imbue(std::locale::classic());
            s << std::hex << std::setfill('0') << std::setw(6) << std::right << (textColor & 0xFFFFFFU);
            repl = s.str();
        }
        output.append(templ, templatePos, match.position() - templatePos);
        output.append(repl);
        templatePos = match.position() + match.length();
    }
    output.append(templ, templatePos);
    return output;
}

auto LatexGenerator::asyncRun(const fs::path& texDir, const std::string& texFileContents) -> Result {
    std::string cmd = this->settings.genCmd;
    GError* err = nullptr;
    const auto&& fail = [&](GenError&& res) -> Result {
        g_error_free(err);
        return res;
    };

    auto texFilePath = (texDir / "tex.tex").string();
    for (auto i = cmd.find(u8"{}"); i != std::string::npos; i = cmd.find(u8"{}", i + texFilePath.length())) {
        cmd.replace(i, 2, texFilePath);
    }
    // Windows note: g_shell_parse_argv assumes POSIX paths, so Windows paths need to be escaped.
    gchar** argv = nullptr;
    if (!g_shell_parse_argv(cmd.c_str(), nullptr, &argv, &err)) {
        return fail({FS(_F("Failed to parse LaTeX generator command: {1}") % err->message)});
    }
    gchar* prog = argv[0];
    if (!prog || !(prog = g_find_program_in_path(prog))) {
        GenError res{FS(_F("Failed to find LaTeX generator program in PATH: {1}") % argv[0])};
        g_strfreev(argv);
        g_error_free(err);
        return res;
    }
    g_free(argv[0]);
    argv[0] = prog;

    if (!g_file_set_contents(texFilePath.c_str(), texFileContents.c_str(), texFileContents.length(), &err)) {
        return fail({FS(_F("Could not save .tex file: {1}") % err->message)});
    }

    auto flags = static_cast<GSubprocessFlags>(G_SUBPROCESS_FLAGS_STDOUT_SILENCE | G_SUBPROCESS_FLAGS_STDERR_SILENCE);
    GSubprocessLauncher* launcher = g_subprocess_launcher_new(flags);
    g_subprocess_launcher_set_cwd(launcher, texDir.u8string().c_str());
    auto* proc = g_subprocess_launcher_spawnv(launcher, argv, &err);

    std::string progName(prog);
    g_strfreev(argv);
    g_object_unref(launcher);

    if (proc) {
        return {proc};
    } else {
        return fail(
                {FS(_F("Could not start {1}: {2} (exit code: {3})") % progName.c_str() % err->message % err->code)});
    }
}
