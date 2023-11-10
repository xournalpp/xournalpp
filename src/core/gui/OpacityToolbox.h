#pragma once

#include <vector>

#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>  // for GtkOverlay

#include "control/ToolHandler.h"    // for ToolHandler
#include "util/Color.h"             // for Color
#include "util/raii/GObjectSPtr.h"  // for GObjectSPtr

#include "FloatingToolbox.h"

class MainWindow;

class OpacityToolbox {

public:
    OpacityToolbox(MainWindow* theMainWindow, GtkOverlay* overlay);
    OpacityToolbox& operator=(const OpacityToolbox&) = delete;
    OpacityToolbox(const OpacityToolbox&) = delete;
    OpacityToolbox& operator=(OpacityToolbox&&) = delete;
    OpacityToolbox(OpacityToolbox&&) = delete;
    ~OpacityToolbox();

    void update();

private:
    /// Returns true if the toolbox is currently hidden.
    bool isHidden() const;

    void hide();
    void showAt(GtkWidget* colorWidget);

    void updateEnabled();
    void updateColor();
    void updatePreviewImage();
    void updateSelectedColorItem();
    void updateScaleValue();

    void setColorWidget(GtkWidget* colorWidget);

    static void changeValue(GtkRange* range, GtkScrollType scroll, gdouble value, OpacityToolbox* self);

    static gboolean getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* allocation,
                                       OpacityToolbox* self);

    static bool handleLeave(GtkEventController* eventController, OpacityToolbox* self);

public:
    bool isEnabled();

private:
    MainWindow* theMainWindow;
    ToolHandler* toolHandler = nullptr;

    /// The overlay that the toolbox should be displayed in.
    xoj::util::GObjectSPtr<GtkOverlay> overlay;

    Color color;
    GtkWidget* colorWidget = nullptr;
    bool enabled;

    xoj::util::WidgetSPtr widget;
    GtkAllocation allocation;

    void updateOpacityToolboxSizeAllocation();
    void updateOpacityToolboxAllocation(GtkWidget* colorWidget);
    void updateOpacityToolboxAllocationX(GtkAllocation colorWidgetAlloc, const int toolbox_width,
                                         const bool isColorItemTooFarLeft, const bool isColorItemTooFarRight,
                                         const int overlap_offset_value);
    void updateOpacityToolboxAllocationY(GtkAllocation colorWidgetAlloc, const int toolbox_height,
                                         const bool isColorItemTooFarLeft, const bool isColorItemTooFarRight,
                                         const bool isColorItemTooFarBottom, const int overlap_offset_value);

    // For enter/leave event on the widget
    xoj::util::GObjectSPtr<GtkEventController> enterLeaveController;

public:
    friend class FloatingToolbox;
    friend class ColorToolItem;
};
