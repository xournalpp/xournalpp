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

#include "filesystem.h"

class LatexSettings {
public:
    bool autoCheckDependencies{true};
    fs::path globalTemplatePath{};
#ifdef __APPLE__
    std::string genCmd{"/Library/TeX/texbin/pdflatex -halt-on-error -interaction=nonstopmode '{}'"};
#else
    std::string genCmd{"pdflatex -halt-on-error -interaction=nonstopmode '{}'"};
#endif
};
