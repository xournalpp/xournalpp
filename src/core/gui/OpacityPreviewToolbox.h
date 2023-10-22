#pragma once

#include <vector>

#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>  // for GtkOverlay

#include "control/ToolHandler.h"               // for ToolHandler
#include "gui/toolbarMenubar/ColorToolItem.h"  // for ColorToolItem
#include "util/Color.h"                        // for Color
#include "util/raii/GObjectSPtr.h"             // for GObjectSPtr

#include "FloatingToolbox.h"

class MainWindow;

class OpacityPreviewToolbox {

public:
    OpacityPreviewToolbox(MainWindow* theMainWindow, GtkOverlay* overlay);
    OpacityPreviewToolbox& operator=(const OpacityPreviewToolbox&) = delete;
    OpacityPreviewToolbox(const OpacityPreviewToolbox&) = delete;
    OpacityPreviewToolbox& operator=(OpacityPreviewToolbox&&) = delete;
    OpacityPreviewToolbox(OpacityPreviewToolbox&&) = delete;
    ~OpacityPreviewToolbox();

public:
    void update();

private:
    /**
     * Toolbox contains opacity preview image and slider.
     * It differs from eventbox which is a small invisible
     * widget positioned over the selected color item
     * that shows or hides toolbox, when being hovered.
     */
    void showToolbox();
    void hideToolbox();
    void updateColor();
    void updatePreviewImage();
    void updateSelectedColorItem();
    void updateScaleValue();
    bool isEnabled();

    void resetEventBoxes();

    /// Returns true if the toolbox is currently hidden.
    bool isHidden() const;

    static void changeValue(GtkRange* range, GtkScrollType scroll, gdouble value, OpacityPreviewToolbox* self);

    static gboolean getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* allocation,
                                       OpacityPreviewToolbox* self);

    static gboolean leaveOpacityToolbox(GtkWidget* opacityToolbox, GdkEventCrossing* event,
                                        OpacityPreviewToolbox* self);

    static gboolean enterEventBox(GtkWidget* eventBox, GdkEventCrossing* event, OpacityPreviewToolbox* self);
    static gboolean leaveEventBox(GtkWidget* eventBox, GdkEventCrossing* event, OpacityPreviewToolbox* self);
    static bool isPointerOverWidget(gint pointer_x_root, gint pointer_y_root, GtkWidget* widget);

private:
    MainWindow* theMainWindow;
    ToolHandler* toolHandler = nullptr;

    /// The overlay that the toolbox should be displayed in.
    xoj::util::GObjectSPtr<GtkOverlay> overlay;

    Color color;

    struct EventBox {
        // The purpose of this small GtkEventBox is to detect when the mouse enters or leaves
        // the area of the selected ColorToolItem. While it may be possible to directly implement
        // a leave/enter signal handler in the ColorToolItem itself, doing so would require establishing
        // a signal connection for every ColorToolItem individually.
        //
        // By using this GtkEventBox, we can centralize the handling of mouse enter/leave
        // events in one place. This simplifies the code and avoids the need for extra
        // signal connections for each ColorToolItem.
        GtkWidget* widget;
        GtkAllocation allocation;
        const ColorToolItem* item;
        bool inFloatingToolbox;
    };

    // The selected color can be represented by several ColorToolItems on different toolbars.
    // An EventBox must be created for each ColorToolItem of the selected color.
    std::vector<EventBox> eventBoxes;

    struct {
        GtkWidget* widget;
        GtkAllocation allocation;

        // The eventBox that is currently in use to show/hide the opacity toolbox
        std::vector<OpacityPreviewToolbox::EventBox>::iterator eventBox;
    } opacityPreviewToolbox;

    void initEventBox(EventBox& eventBox, ColorToolItem* colorItem, int index);
    void updateEventBoxAllocation(EventBox& eventBox);
    void updateOpacityToolboxSizeAllocation();
    void updateOpacityToolboxAllocation(EventBox eventBox);
    void updateOpacityToolboxAllocationX(EventBox& eventBox, const int toolbox_width, const bool isColorItemTooFarLeft,
                                         const bool isColorItemTooFarRight, const int overlap_offset_value);
    void updateOpacityToolboxAllocationY(EventBox& eventBox, const int toolbox_height, const bool isColorItemTooFarLeft,
                                         const bool isColorItemTooFarRight, const bool isColorItemTooFarBottom,
                                         const int overlap_offset_value);
    std::vector<OpacityPreviewToolbox::EventBox>::iterator findEventBox(GtkWidget* eventBoxWidget);

public:
    friend class FloatingToolbox;

private:
    void hideEventBoxesInFloatingToolBox();
    void showEventBoxesInFloatingToolBox();
};
