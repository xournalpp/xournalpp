#include "LatexGenerator.h"

#include <algorithm>    // for transform
#include <map>          // for map
#include <regex>        // for smatch, sregex_iterator
#include <sstream>      // for ostringstream
#include <string_view>  // for string_view

#include <glib.h>     // for GError, gchar, g_error_free
#include <poppler.h>  // for g_object_unref

#include "control/settings/LatexSettings.h"  // for LatexSettings
#include "util/PathUtil.h"                   // for getLongPath
#include "util/PlaceholderString.h"          // for PlaceholderString
#include "util/Util.h"                       // for Util
#include "util/i18n.h"                       // for FS, _F
#include "util/raii/GLibGuards.h"            // for GErrorGuard, GStrvGuard
#include "util/raii/GObjectSPtr.h"           // for GObjectSptr
#include "util/safe_casts.h"                 // for as_signed


using namespace xoj::util;

LatexGenerator::LatexGenerator(const LatexSettings& settings): settings(settings) {}

auto LatexGenerator::templateSub(const std::string& input, const std::string& templ, const Color textColor)
        -> std::string {
    std::map<std::string, std::string> vars;
    std::vector<std::string> bodyLines;

    const static std::regex directiveRe(R"(^%xpp:([A-Za-z_][A-Za-z0-9_]*)=(.*)$)");

    std::istringstream inputStream(input);
    std::string line;
    while (std::getline(inputStream, line)) {
        std::smatch match;
        if (std::regex_match(line, match, directiveRe)) {
            std::string key = match[1].str();
            std::string value = match[2].str();

            size_t start = value.find_first_not_of(" \t\r\n");
            if (start != std::string::npos) {
                size_t end = value.find_last_not_of(" \t\r\n");
                value = value.substr(start, end - start + 1);
            } else {
                value.clear();
            }

            std::transform(key.begin(), key.end(), key.begin(), ::toupper);

            if (key != "TOOL_INPUT" && key != "TEXT_COLOR") {
                vars[key] = value;
            }
        } else {
            bodyLines.push_back(line);
        }
    }

    std::string strippedBody;
    for (const auto& l: bodyLines) {
        if (!strippedBody.empty())
            strippedBody += '\n';
        strippedBody += l;
    }

    vars["TOOL_INPUT"] = strippedBody;
    vars["TEXT_COLOR"] = Util::rgb_to_hex_string(textColor).substr(1);

    const static std::regex substRe(R"(%%XPP_[A-Z][A-Z0-9_]*%%)");
    std::string output;
    output.reserve(templ.length());
    size_t templatePos = 0;

    for (std::sregex_iterator it(templ.begin(), templ.end(), substRe); it != std::sregex_iterator{}; ++it) {
        std::smatch match = *it;
        std::string placeholder = match.str();

        constexpr size_t kPrefixLen = 6;  // "%%XPP_"
        constexpr size_t kSuffixLen = 2;  // "%%"
        std::string key = placeholder.substr(kPrefixLen, placeholder.length() - kPrefixLen - kSuffixLen);

        output.append(templ, templatePos, as_unsigned(match.position()) - templatePos);

        auto itVar = vars.find(key);
        if (itVar != vars.end()) {
            output.append(itVar->second);
        }

        templatePos = as_unsigned(match.position() + match.length());
    }

    output.append(templ, templatePos);
    return output;
}

auto LatexGenerator::asyncRun(const fs::path& texDir, const std::string& texFileContents) -> Result {
    std::string cmd = this->settings.genCmd;
    GErrorGuard err{};
    std::string texFilePathOSEncoding = Util::GFilename(Util::getLongPath(texDir) / "tex.tex").c_str();

    for (auto i = cmd.find("{}"); i != std::string::npos; i = cmd.find("{}", i + texFilePathOSEncoding.length())) {
        cmd.replace(i, 2, texFilePathOSEncoding);
    }
    // Todo (rolandlo): is this a todo?
    // Windows note: g_shell_parse_argv assumes POSIX paths, so Windows paths need to be escaped.
    GStrvGuard argv{};
    if (!g_shell_parse_argv(cmd.c_str(), nullptr, out_ptr(argv), out_ptr(err))) {
        return GenError{FS(_F("Failed to parse LaTeX generator command: {1}") % err->message)};
    }
    gchar* prog = argv.get()[0];
    if (!prog || !(prog = g_find_program_in_path(prog))) {
        if (Util::isFlatpakInstallation()) {
            return GenError{
                    FS(_F("Failed to find LaTeX generator program in PATH: {1}\n\nSince installation is detected "
                          "within Flatpak, you need to install the Flatpak freedesktop Tex Live extension. For "
                          "example, by running:\n\n$ flatpak install flathub org.freedesktop.Sdk.Extension.texlive") %
                       argv.get()[0])};
        } else {
            return GenError{FS(_F("Failed to find LaTeX generator program in PATH: {1}") % argv.get()[0])};
        }
    }
    g_free(argv.get()[0]);
    argv.get()[0] = prog;

    if (!g_file_set_contents(texFilePathOSEncoding.c_str(), texFileContents.c_str(), as_signed(texFileContents.size()),
                             out_ptr(err))) {
        return GenError({FS(_F("Could not save .tex file: {1}") % err->message)});
    }

    auto flags = static_cast<GSubprocessFlags>(G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE);
    xoj::util::GObjectSPtr<GSubprocessLauncher> launcher(g_subprocess_launcher_new(flags), xoj::util::adopt);
    g_subprocess_launcher_set_cwd(launcher.get(), Util::GFilename(texDir).c_str());
    auto* proc = g_subprocess_launcher_spawnv(launcher.get(), argv.get(), out_ptr(err));

    if (proc) {
        return {proc};
    }
    std::ostringstream ss;
    for (char** iter = argv.get(); iter != nullptr && *iter != nullptr; ++iter) {
        ss << std::string_view(*iter) << ", ";
    }
    return GenError({FS(_F("Could not start {1}: {2} (exit code: {3})") % ss.str() % err->message % err->code)});
}
