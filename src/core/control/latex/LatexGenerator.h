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

#include <string>   // for string
#include <variant>  // for variant

#include <gio/gio.h>  // for GSubprocess

#include "util/Color.h"  // for Color

#include "filesystem.h"  // for path

class LatexSettings;

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
     * The resultant process will have its standard error and (original) standard output
     * combined into a single standard out stream.
     */
    Result asyncRun(const fs::path& texDir, const std::string& texFileContents);

    /**
     * Instantiate the LaTeX template.
     */
    static std::string templateSub(const std::string& input, const std::string& templ, Color textColor);

private:
    const LatexSettings& settings;
};
