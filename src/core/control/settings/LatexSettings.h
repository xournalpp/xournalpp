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

class LatexSettings {
public:
    bool autoCheckDependencies{true};
    std::string defaultText{"x^2"};
    fs::path globalTemplatePath{};
#ifdef __APPLE__
    std::string genCmd{"/Library/TeX/texbin/pdflatex -halt-on-error -interaction=nonstopmode '{}'"};
#else
    std::string genCmd{"pdflatex -halt-on-error -interaction=nonstopmode '{}'"};
#endif

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
};
