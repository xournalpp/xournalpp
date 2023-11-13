#include "OpacityToolbox.h"

#include <cmath>  // for std::round

#include <cairo.h>        // for cairo_set_operator, cairo_rectangle, cairo_...
#include <glib-object.h>  // for G_CALLBACK, g_signal_connect
#include <glib.h>         // for gdouble

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "gui/FloatingToolbox.h"
#include "gui/XournalView.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "util/Color.h"
#include "util/GtkUtil.h"
#include "util/gtk4_helper.h"
#include "util/raii/CairoWrappers.h"
#include "util/raii/GObjectSPtr.h"

#include "MainWindow.h"


static int percentToByte(double percent) { return round_cast<int>(percent * 2.55); }
static double byteToPercent(int byte) { return byte / 2.55; }

OpacityToolbox::OpacityToolbox(MainWindow* theMainWindow, GtkOverlay* overlay):
        theMainWindow(theMainWindow), overlay(overlay, xoj::util::refsink) {
    GtkWidget* oWidget = theMainWindow->get("opacityToolbox");
    this->widget.reset(oWidget, xoj::util::refsink);

    gtk_overlay_add_overlay(overlay, oWidget);

    g_signal_connect(overlay, "get-child-position", G_CALLBACK(this->getOverlayPosition), this);
    g_signal_connect(theMainWindow->get("opacityToolboxScaleAlpha"), "change-value", G_CALLBACK(this->changeValue),
                     this);

#if GTK_MAJOR_VERSION == 3
    GtkEventController* controller = gtk_event_controller_motion_new(oWidget);
    gtk_widget_add_events(this->widget.get(), GDK_ENTER_NOTIFY_MASK);
    gtk_widget_add_events(this->widget.get(), GDK_LEAVE_NOTIFY_MASK);
#else
    GtkEventController* controller = gtk_event_controller_motion_new();
    gtk_widget_add_controller(oWidget, controller);
#endif
    this->enterLeaveController.reset(controller, xoj::util::refsink);
    gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_TARGET);

    g_signal_connect(controller, "leave", G_CALLBACK(handleLeave), this);
}

void OpacityToolbox::changeValue(GtkRange* range, GtkScrollType scroll, gdouble value, OpacityToolbox* self) {
    gtk_range_set_value(range, value);
    gdouble rangedValue = gtk_range_get_value(range);
    self->color.alpha = static_cast<uint8_t>(percentToByte(rangedValue));
    self->updatePreviewImage();

    ToolHandler* toolHandler = self->toolHandler;

    switch (toolHandler->getToolType()) {
        case TOOL_SELECT_PDF_TEXT_RECT:
        case TOOL_SELECT_PDF_TEXT_LINEAR:
            toolHandler->setSelectPDFTextMarkerOpacity(self->color.alpha);
            break;
        case TOOL_PEN:
            toolHandler->setPenFill(self->color.alpha);
            break;
        case TOOL_HIGHLIGHTER:
            toolHandler->setHighlighterFill(self->color.alpha);
            break;
        case TOOL_SELECT_RECT:
        case TOOL_SELECT_REGION:
        case TOOL_SELECT_MULTILAYER_RECT:
        case TOOL_SELECT_MULTILAYER_REGION:
        case TOOL_SELECT_OBJECT:
            self->theMainWindow->getXournal()->getSelection()->setFill(self->color.alpha, self->color.alpha);
            break;
        default:
            toolHandler->setColor(self->color, false);
            break;
    }
}

bool OpacityToolbox::handleLeave(GtkEventController* eventController, OpacityToolbox* self) {
    if (!self->isHidden()) {

        // Leave signal will be emitted if entering another child widget (GtkScale in the present case)
        // So, this condition is needed to check if the pointer is truly outside of the opacity toolbox.
        if (!xoj::util::gtk::isEventOverWidget(eventController, self->widget.get())) {
            // Hide the floating toolbox if the mouse is not over it.
            // This handles the case where the pointer entered the opacity toolbox
            // for a ColorToolItem within the floating toolbox.
            FloatingToolbox* oFloatingToolbox = self->theMainWindow->getFloatingToolbox();

            if (!xoj::util::gtk::isEventOverWidget(eventController, oFloatingToolbox->floatingToolbox)) {
                oFloatingToolbox->hide();
            }
            self->hide();
        }
    }
    return true;
}

void OpacityToolbox::updateEnabled() {
    bool result;

    switch (this->toolHandler->getToolType()) {
        case TOOL_PEN:
            result = toolHandler->getPenFillEnabled() ? true : false;
            break;
        case TOOL_HIGHLIGHTER:
            result = toolHandler->getHighlighterFillEnabled() ? true : false;
            break;
        case TOOL_SELECT_RECT:
        case TOOL_SELECT_REGION:
        case TOOL_SELECT_MULTILAYER_RECT:
        case TOOL_SELECT_MULTILAYER_REGION:
        case TOOL_SELECT_OBJECT:
        case TOOL_SELECT_PDF_TEXT_RECT:
        case TOOL_SELECT_PDF_TEXT_LINEAR:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    this->enabled = result;
}

bool OpacityToolbox::isEnabled() { return this->enabled; }

void OpacityToolbox::updateColor() {
    this->color = this->toolHandler->getColor();
    ToolType tooltype = this->toolHandler->getToolType();

    switch (tooltype) {
        case TOOL_PEN:
            this->color.alpha = static_cast<uint8_t>(toolHandler->getPenFill());
            break;
        case TOOL_HIGHLIGHTER:
            this->color.alpha = static_cast<uint8_t>(toolHandler->getHighlighterFill());
            break;
        default:
            break;
    }
}

void OpacityToolbox::setColorWidget(GtkWidget* colorWidget) {
    if (this->isEnabled()) {
        this->colorWidget = colorWidget;
    }
}

void OpacityToolbox::update() {
    if (this->toolHandler == nullptr) {
        this->toolHandler = theMainWindow->getControl()->getToolHandler();
    }

    this->updateEnabled();

    if (this->isEnabled()) {
        this->updateColor();
        this->updateScaleValue();
        this->updatePreviewImage();

        if (this->colorWidget != nullptr) {
            this->showAt(this->colorWidget);
            this->colorWidget = nullptr;
        }
    }
}

void OpacityToolbox::updateOpacityToolboxSizeAllocation() {
    // Get existing width and height
    GtkRequisition natural;

    gtk_widget_get_preferred_size(this->widget.get(), nullptr, &natural);

    this->allocation.width = natural.width;
    this->allocation.height = natural.height;
}

/**
 * Adjust the position of the opacity toolbox so that it sits below, above, on the right or
 * on the left of the selected color item, in a manner that makes it fully visible.
 */
void OpacityToolbox::updateOpacityToolboxAllocation(GtkWidget* colorWidget) {
    this->updateOpacityToolboxSizeAllocation();

    // If the toolbox will go out of the window, then we'll flip the corresponding directions.
    GtkWidget* overlayWidget = GTK_WIDGET(overlay.get());

    GtkAllocation windowAlloc{};
    windowAlloc.width = gtk_widget_get_width(overlayWidget);
    windowAlloc.height = gtk_widget_get_height(overlayWidget);

    GtkAllocation colorWidgetAlloc{};
    colorWidgetAlloc.width = gtk_widget_get_width(colorWidget);
    colorWidgetAlloc.height = gtk_widget_get_height(colorWidget);


    const graphene_point_t colorWidgetOrigin = {0, 0};
    graphene_point_t colorWidgetOriginInOverlayCoords;

    gtk_widget_compute_point(colorWidget, GTK_WIDGET(this->overlay.get()), &colorWidgetOrigin,
                             &colorWidgetOriginInOverlayCoords);

    colorWidgetAlloc.x = static_cast<int>(colorWidgetOriginInOverlayCoords.x);
    colorWidgetAlloc.y = static_cast<int>(colorWidgetOriginInOverlayCoords.y);

    // Make sure the "OpacityToolbox" is fully displayed.
    //        const int gap = 5;
    const int gap = 0;

    bool isColorItemTooFarLeft = colorWidgetAlloc.x - this->allocation.width - gap < 0;
    bool isColorItemTooFarRight =
            colorWidgetAlloc.x + colorWidgetAlloc.width + allocation.width + gap > windowAlloc.width;
    bool isColorItemTooFarBottom =
            colorWidgetAlloc.y + colorWidgetAlloc.height + this->allocation.height + gap > windowAlloc.height;

    // Ensure an overlap between the selected ColorToolItem and the opacity toolbox
    // for handling the "notify-leave-event" signal, so that the user can leave
    // the selected ColorToolItem and enter the opacity toolbox without making it
    // disappear.
    int overlap_offset_value = 5;

    // Increase overlap in corners for an improved user experience,
    // especially due to the rounded corners of the opacity toolbox.
    if (isColorItemTooFarBottom && (isColorItemTooFarRight || isColorItemTooFarLeft)) {
        overlap_offset_value = 15;
    }

    this->updateOpacityToolboxAllocationX(colorWidgetAlloc, this->allocation.width, isColorItemTooFarLeft,
                                          isColorItemTooFarRight, overlap_offset_value);
    this->updateOpacityToolboxAllocationY(colorWidgetAlloc, this->allocation.height, isColorItemTooFarLeft,
                                          isColorItemTooFarRight, isColorItemTooFarBottom, overlap_offset_value);
}

/**
 * Adjust the horizontal position of the opacity toolbox so that it sits on the right, on the left
 * or is vertically centered with the selected color item, in a manner that makes it fully visible.
 */
void OpacityToolbox::updateOpacityToolboxAllocationX(GtkAllocation colorWidgetAlloc, const int toolbox_width,
                                                     const bool isColorItemTooFarLeft,
                                                     const bool isColorItemTooFarRight,
                                                     const int overlap_offset_value) {
    int overlap_offset_x = 0;

    if (isColorItemTooFarLeft) {
        // Position the opacity toolbox to the right of the ColorToolItem
        this->allocation.x = colorWidgetAlloc.x + colorWidgetAlloc.width;
        overlap_offset_x = -overlap_offset_value;
    } else if (isColorItemTooFarRight) {
        // Position the opacity toolbox to the left of the ColorToolItem
        this->allocation.x = colorWidgetAlloc.x - toolbox_width;
        overlap_offset_x = overlap_offset_value;
    } else {
        // Centers vertically the opacity toolbox with the selected ColorToolItem
        int offset_x = static_cast<int>(std::round(colorWidgetAlloc.width - toolbox_width) / 2);
        this->allocation.x = colorWidgetAlloc.x + offset_x;
    }

    this->allocation.x += overlap_offset_x;
}

/**
 * Adjust the vertical position of the opacity toolbox so that it sits below, above or
 * horizontally centered with the selected color item, in a manner that makes it fully visible.
 */
void OpacityToolbox::updateOpacityToolboxAllocationY(GtkAllocation colorWidgetAlloc, const int toolbox_height,
                                                     const bool isColorItemTooFarLeft,
                                                     const bool isColorItemTooFarRight,
                                                     const bool isColorItemTooFarBottom,
                                                     const int overlap_offset_value) {
    int overlap_offset_y = 0;

    if (isColorItemTooFarBottom) {
        // Position the opacity toolbox to the top of the ColorToolItem
        this->allocation.y = colorWidgetAlloc.y - toolbox_height;
        overlap_offset_y = overlap_offset_value;
    } else if (isColorItemTooFarLeft || isColorItemTooFarRight) {
        // Centers horizontally the opacity toolbox with the selected ColorToolItem
        int offset_y = static_cast<int>(std::round(colorWidgetAlloc.height - toolbox_height) / 2);
        this->allocation.y = colorWidgetAlloc.y + offset_y;
    } else {
        // Position the opacity toolbox below the ColorToolIltem
        this->allocation.y = colorWidgetAlloc.y + colorWidgetAlloc.height;
        overlap_offset_y = -overlap_offset_value;
    }

    this->allocation.y += overlap_offset_y;
}

void OpacityToolbox::updateScaleValue() {
    GtkRange* rangeWidget = (GtkRange*)this->theMainWindow->get("opacityToolboxScaleAlpha");
    gtk_range_set_value(rangeWidget, byteToPercent(this->color.alpha));
}

static bool inline useBorderForPreview(ToolType tooltype) {
    switch (tooltype) {
        case TOOL_PEN:
            return true;
        default:
            return false;
    }
}

const int PREVIEW_WIDTH = 70;
const int PREVIEW_HEIGHT = 50;
const int PREVIEW_BORDER = 10;

void OpacityToolbox::updatePreviewImage() {
    bool addBorder = useBorderForPreview(toolHandler->getToolType());

    xoj::util::CairoSurfaceSPtr surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, PREVIEW_WIDTH, PREVIEW_HEIGHT),
                                        xoj::util::adopt);
    xoj::util::CairoSPtr cairo(cairo_create(surface.get()), xoj::util::adopt);
    cairo_t* cr = cairo.get();

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb(cr, 255, 255, 255);
    cairo_paint(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    Util::cairo_set_source_argb(cr, this->color);
    cairo_rectangle(cr, PREVIEW_BORDER, PREVIEW_BORDER, PREVIEW_WIDTH - PREVIEW_BORDER * 2,
                    PREVIEW_HEIGHT - PREVIEW_BORDER * 2);
    cairo_fill(cr);

    if (addBorder) {
        cairo_set_line_width(cr, 5);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        Color borderColor = this->color;
        borderColor.alpha = 255;
        Util::cairo_set_source_argb(cr, borderColor);
        cairo_rectangle(cr, PREVIEW_BORDER, PREVIEW_BORDER, PREVIEW_WIDTH - PREVIEW_BORDER * 2,
                        PREVIEW_HEIGHT - PREVIEW_BORDER * 2);
        cairo_stroke(cr);
    }

    xoj::util::GObjectSPtr<GdkPixbuf> pixbuf(
            gdk_pixbuf_get_from_surface(surface.get(), 0, 0, PREVIEW_WIDTH, PREVIEW_HEIGHT), xoj::util::adopt);
    gtk_image_set_from_pixbuf(GTK_IMAGE(theMainWindow->get("opacityToolboxImg")), pixbuf.get());
}

OpacityToolbox::~OpacityToolbox() = default;

void OpacityToolbox::showAt(GtkWidget* colorWidget) {
    gtk_widget_hide(this->widget.get());  // force showing in new position
    gtk_widget_show_all(this->widget.get());

    this->updateOpacityToolboxAllocation(colorWidget);
}

void OpacityToolbox::hide() {
    if (isHidden())
        return;

    gtk_widget_hide(this->widget.get());
}

auto OpacityToolbox::getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* allocation,
                                        OpacityToolbox* self) -> gboolean {
    if (!self->isEnabled() || self->isHidden()) {
        return false;
    }

    if (widget == self->widget.get()) {
        self->updateOpacityToolboxSizeAllocation();

        allocation->x = self->allocation.x;
        allocation->y = self->allocation.y;
        allocation->width = self->allocation.width;
        allocation->height = self->allocation.height;
        return true;
    }
    return false;
}

bool OpacityToolbox::isHidden() const { return !gtk_widget_is_visible(this->widget.get()); }
