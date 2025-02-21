/*
 * Xournal++
 *
 * The thickness selection dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>

#include "control/ToolEnums.h"
#include "util/raii/GtkWindowUPtr.h"

class GladeSearchpath;

namespace xoj::popup {
class CustomThicknessDialog {
public:
    CustomThicknessDialog(GladeSearchpath* gladeSearchPath, double thickness, CustomToolSizeFeature type,
                          std::function<void(double, CustomToolSizeFeature)> callback);
    ~CustomThicknessDialog();

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    xoj::util::GtkWindowUPtr window;
    GtkRange* sizeRange;

    CustomToolSizeFeature customToolSizeFeature;
    std::function<void(double, CustomToolSizeFeature)> callback;
};
};  // namespace xoj::popup
