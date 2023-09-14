/*
 * Xournal++
 *
 * The about dialog
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
class FillOpacityDialog {
public:
    FillOpacityDialog(GladeSearchpath* gladeSearchPath, int alpha, ToolType type,
                      std::function<void(int, ToolType)> callback);
    ~FillOpacityDialog();

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    void setPreviewImage(int alpha);

private:
    xoj::util::GtkWindowUPtr window;
    GtkImage* previewImage;
    GtkRange* alphaRange;

    ToolType toolType;
    std::function<void(int, ToolType)> callback;
};
};  // namespace xoj::popup
