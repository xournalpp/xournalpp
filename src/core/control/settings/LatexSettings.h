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

class LatexSettings {
public:
    bool autoCheckDependencies{true};
    std::string defaultText{"x^2"};
    fs::path globalTemplatePath{};
    std::string genCmd{defaultPdflatexPath};
    std::string genArgs{" -halt-on-error -interaction=nonstopmode '{}'"};

    enum class type_t { pdflatex, typst, custom };
    type_t type{type_t::pdflatex};

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

    void applyTemplate(type_t templateType);
};
