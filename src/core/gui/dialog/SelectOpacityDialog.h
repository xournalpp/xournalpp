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
class SelectOpacityDialog {
public:
    SelectOpacityDialog(GladeSearchpath* gladeSearchPath, int alpha, OpacityFeature type,
                        std::function<void(int, OpacityFeature)> callback);
    ~SelectOpacityDialog();

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    void setPreviewImage(int alpha);

private:
    xoj::util::GtkWindowUPtr window;
    GtkPicture* previewImage;
    GtkRange* alphaRange;

    OpacityFeature opacityFeature;
    std::function<void(int, OpacityFeature)> callback;
};
};  // namespace xoj::popup
