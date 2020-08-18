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
    std::string genCmd{"pdflatex -interaction=nonstopmode '{}'"};
};
