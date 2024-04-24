#include "BaseElementView.h"

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "control/settings/Settings.h"  // for Settings
#include "gui/Shadow.h"                 // for Shadow
#include "util/Color.h"                 // for cairo_set_source_rgbi

#include "BackgroundSelectDialogBase.h"  // for BackgroundSelectDialogBase

constexpr int BORDER_WIDTH = 2;

BaseElementView::BaseElementView(size_t id, BackgroundSelectDialogBase* dlg): dlg(dlg), id(id) {
    this->widget = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(widget),
                                   GtkDrawingAreaDrawFunc(+[](GtkWidget*, cairo_t* cr, int, int, gpointer element) {
                                       static_cast<BaseElementView*>(element)->paint(cr);
                                   }),
                                   this, nullptr);

    auto* ctrl = gtk_gesture_click_new();
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(ctrl));
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(ctrl), GDK_BUTTON_PRIMARY);
    g_signal_connect(ctrl, "pressed",
                     G_CALLBACK(+[](GtkGestureClick* g, gint n_press, gdouble x, gdouble y, gpointer d) {
                         if (n_press == 1) {
                             auto* element = static_cast<BaseElementView*>(d);
                             element->dlg->setSelected(element->id);
                         }
                     }),
                     this);
}

BaseElementView::~BaseElementView() = default;

void BaseElementView::setSelected(bool selected) {
    if (this->selected == selected) {
        return;
    }
    this->selected = selected;

    repaint();
}

void BaseElementView::repaint() {
    if (this->crBuffer) {
        cairo_surface_destroy(this->crBuffer);
        this->crBuffer = nullptr;
    }
    gtk_widget_queue_draw(this->widget);
}

void BaseElementView::paint(cairo_t* cr) {
    GtkAllocation alloc;
    gtk_widget_get_allocation(this->widget, &alloc);

    if (this->crBuffer == nullptr) {
        this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);

        int width = getContentWidth();
        int height = getContentHeight();

        cairo_t* cr2 = cairo_create(this->crBuffer);

        cairo_save(cr2);
        cairo_translate(cr2, Shadow::getShadowTopLeftSize() + BORDER_WIDTH,
                        Shadow::getShadowTopLeftSize() + BORDER_WIDTH);
        paintContents(cr2);
        cairo_restore(cr2);

        cairo_set_operator(cr2, CAIRO_OPERATOR_OVER);

        if (this->selected) {
            // Draw border
            Util::cairo_set_source_rgbi(cr2, dlg->getSettings()->getBorderColor());
            cairo_set_line_width(cr2, BORDER_WIDTH);
            cairo_set_line_cap(cr2, CAIRO_LINE_CAP_BUTT);
            cairo_set_line_join(cr2, CAIRO_LINE_JOIN_BEVEL);

            cairo_rectangle(cr2, Shadow::getShadowTopLeftSize() + BORDER_WIDTH / 2,
                            Shadow::getShadowTopLeftSize() + BORDER_WIDTH / 2, width + BORDER_WIDTH,
                            height + BORDER_WIDTH);

            cairo_stroke(cr2);

            Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize(), Shadow::getShadowTopLeftSize(),
                               width + 2 * BORDER_WIDTH, height + 2 * BORDER_WIDTH);
        } else {
            Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize() + BORDER_WIDTH,
                               Shadow::getShadowTopLeftSize() + BORDER_WIDTH, width, height);
        }

        cairo_destroy(cr2);
    }

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_surface(cr, this->crBuffer, 0, 0);
    cairo_paint(cr);
}

auto BaseElementView::getWidget() -> GtkWidget* {
    updateSize();
    return this->widget;
}

auto BaseElementView::getWidth() -> int {
    calcSize();
    return getContentWidth() + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 2 * BORDER_WIDTH;
}

auto BaseElementView::getHeight() -> int {
    calcSize();
    return getContentHeight() + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 2 * BORDER_WIDTH;
}

void BaseElementView::calcSize() {
    // Not implemented in the base class
}

void BaseElementView::updateSize() { gtk_widget_set_size_request(this->widget, this->getWidth(), this->getHeight()); }
