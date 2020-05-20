/*
 * Xournal++
 *
 * Latex file generator
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <variant>

#include <gio/gio.h>
#include <poppler.h>

#include "control/settings/LatexSettings.h"

#include "Path.h"

class LatexGenerator {
public:
    LatexGenerator(const LatexSettings& settings);
    LatexGenerator(const LatexGenerator&) = delete;
    LatexGenerator& operator=(const LatexGenerator&) = delete;
    LatexGenerator(const LatexGenerator&&) = delete;
    LatexGenerator&& operator=(const LatexGenerator&&) = delete;
    virtual ~LatexGenerator() = default;

    struct GenError {
        std::string message;
    };
    using Result = std::variant<GSubprocess*, GenError>;

    /**
     * Run the LaTeX command asynchronously to generate a preview for the given
     * LaTeX file. The contents of the LaTeX file will be written to "tex.tex"
     * in the given directory.
     */
    Result asyncRun(const Path& texDir, const std::string& texFileContents);

    /**
     * Instantiate the LaTeX template.
     */
    static std::string templateSub(const string& input, const string& templ, const uint32_t textColor);

private:
    const LatexSettings& settings;
};
