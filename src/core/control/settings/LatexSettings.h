/*
 * Xournal++
 *
 * Latex settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>

#include "model/Font.h"

#include "filesystem.h"

#ifdef __APPLE__
constexpr std::string_view defaultPdflatexPath = "/Library/TeX/texbin/pdflatex";
#else
constexpr std::string_view defaultPdflatexPath = "/usr/bin/pdflatex";
#endif

class TemplateSettings {
public:
    bool autoCheckDependencies{true};
    std::string defaultText{"x^2"};
    fs::path globalTemplatePath{};
    fs::path genCmd{defaultPdflatexPath};
    std::string genArgs{" -halt-on-error -interaction=nonstopmode '{}'"};
    std::string genTmpFileExt{"tex"};

    enum type { pdflatex, typst };

    /**
     * LaTeX editor theme. Only used if linked with the GtkSourceView
     * library.
     */
    std::string sourceViewThemeId{};
    bool sourceViewAutoIndent{true};
    bool sourceViewSyntaxHighlight{true};
    bool sourceViewShowLineNumbers{false};

    /**
     * Font to be used by the editor.
     */
    XojFont editorFont{"Monospace", 12};
    bool useCustomEditorFont{false};

    bool editorWordWrap{true};

    bool useExternalEditor{false};
    bool externalEditorAutoConfirm{false};
    std::string externalEditorCmd{};
    std::string temporaryFileExt{"tex"};
};

// use defaults fallback
class LatexSettings: public TemplateSettings {};

// typst template
class TypstSettings: public LatexSettings {
public:
    fs::path genCmd{"typst"};
    std::string genArgs{" c {} tex.pdf"};
    std::string defaultText{"x^2"};
    fs::path globalTemplatePath{};
    std::string genTmpFileExt{"typ"};
};
