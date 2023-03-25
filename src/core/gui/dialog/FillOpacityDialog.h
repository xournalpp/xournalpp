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

#include "util/raii/GtkWindowUPtr.h"

class GladeSearchpath;

namespace xoj::popup {
class FillOpacityDialog {
public:
    FillOpacityDialog(GladeSearchpath* gladeSearchPath, int alpha, bool pen, std::function<void(int, bool)> callback);
    ~FillOpacityDialog();

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    void setPreviewImage(int alpha);

private:
    xoj::util::GtkWindowUPtr window;
    GtkImage* previewImage;
    GtkRange* alphaRange;

    bool pen;
    std::function<void(int, bool)> callback;
};
};  // namespace xoj::popup
