#include "LatexSettings.h"
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

#include <string>

#include "model/Font.h"

#include "filesystem.h"

void LatexSettings::applyTemplate(const type_t templateType) {
    this->type = templateType;
    switch (templateType) {
        case type_t::pdflatex:
            // use defaults fallback

            this->temporaryFileExt = "tex";

            this->genCmd = defaultPdflatexPath;
            this->genArgs = " -halt-on-error -interaction=nonstopmode '{}'";

            this->defaultText = "x^2";
            // TODO:
            // this->globalTemplatePath{};
            break;

        case type_t::typst:
            // typst template

            this->temporaryFileExt = "typ";

            this->genCmd = "/usr/bin/typst";
            this->genArgs = " c '{}' tex.pdf";

            this->defaultText = "x^2";
            // TODO:
            // this->globalTemplatePath{};
            break;

        case type_t::custom:
            this->temporaryFileExt = "tex";
            break;
    }
}
