#include "XournalWidget.h"

#include <algorithm>  // for max
#include <cmath>      // for NAN
#include <optional>   // for optional
#include <vector>     // for vector

#include <cairo.h>    // for cairo_restore, cairo_save
#include <gdk/gdk.h>  // for GdkRectangle, GdkWindowAttr

#include "control/Control.h"                // for Control
#include "control/settings/Settings.h"      // for Settings
#include "control/tools/EditSelection.h"    // for EditSelection
#include "gui/Layout.h"                     // for Layout
#include "gui/LegacyRedrawable.h"           // for Redrawable
#include "gui/PageView.h"                   // for XojPageView
#include "gui/Shadow.h"                     // for Shadow
#include "gui/XournalView.h"                // for XournalView
#include "gui/inputdevices/InputContext.h"  // for InputContext
#include "gui/scroll/ScrollHandling.h"      // for ScrollHandling
#include "util/Color.h"                     // for cairo_set_source_rgbi
#include "util/Rectangle.h"                 // for Rectangle
#include "util/raii/CairoWrappers.h"

#include "config-debug.h"  // for DEBUG_DRAW_WIDGET

/*
 * Declares:
 *      static void gtk_xournal_class_init(GtkXournalClass*);
 *      static void gtk_xournal_init(GtkXournal*);
 * Defines
 *      gtk_xournal_parent_class (pointer to GtkWidgetClass instance)
 *      GType gtk_xournal_get_type();
 */
G_DEFINE_TYPE(GtkXournal, gtk_xournal, GTK_TYPE_WIDGET)

static void gtk_xournal_measure(GtkWidget* widget, GtkOrientation orientation, int for_size, int* minimum, int* natural,
                                int* minimum_baseline, int* natural_baseline);
static void gtk_xournal_snapshot(GtkWidget* widget, GtkSnapshot* sn);
static void gtk_xournal_dispose(GObject* object);

auto gtk_xournal_new(XournalView* view, InputContext* inputContext) -> GtkWidget* {
    GtkXournal* xoj = GTK_XOURNAL(g_object_new(gtk_xournal_get_type(), nullptr));
    xoj->view = view;
    xoj->scrollHandling = inputContext->getScrollHandling();
    xoj->layout = new Layout(view, inputContext->getScrollHandling());
    xoj->selection = nullptr;
    xoj->input = inputContext;

    xoj->input->connect(GTK_WIDGET(xoj));

    gtk_widget_set_hexpand(GTK_WIDGET(xoj), true);
    gtk_widget_set_vexpand(GTK_WIDGET(xoj), true);

    return GTK_WIDGET(xoj);
}

static void gtk_xournal_class_init(GtkXournalClass* cptr) {
    auto* widget_class = reinterpret_cast<GtkWidgetClass*>(cptr);

    widget_class->measure = gtk_xournal_measure;
    widget_class->snapshot = gtk_xournal_snapshot;

#ifdef DEBUG_DRAW_WIDGET
    widget_class->queue_draw_region = +[](GtkWidget* w, const cairo_region_t* reg) {
        cairo_rectangle_int_t r;
        cairo_region_get_extents(reg, &r);
        auto width = gtk_widget_get_allocated_width(w);
        auto height = gtk_widget_get_allocated_height(w);

        auto widthp = gtk_widget_get_allocated_width(gtk_widget_get_parent(w));
        auto heightp = gtk_widget_get_allocated_height(gtk_widget_get_parent(w));
        printf("   * queue_draw_region: %d x %d + (%d ; %d) out of %d x %d   parent: %d x %d\n", r.width, r.height, r.x,
               r.y, width, height, widthp, heightp);
        GTK_WIDGET_CLASS(gtk_xournal_parent_class)->queue_draw_region(w, reg);
    };
#endif

    G_OBJECT_CLASS(cptr)->dispose = gtk_xournal_dispose;
}

auto gtk_xournal_get_visible_area(GtkWidget* widget, const XojPageView* p) -> xoj::util::Rectangle<double>* {
    if (!p || !p->isVisible()) {
        return nullptr;
    }

    g_return_val_if_fail(widget != nullptr, nullptr);
    g_return_val_if_fail(GTK_IS_XOURNAL(widget), nullptr);

    GtkXournal* xournal = GTK_XOURNAL(widget);

    GtkAdjustment* vadj = xournal->scrollHandling->getVertical();
    GtkAdjustment* hadj = xournal->scrollHandling->getHorizontal();

    GdkRectangle r2;
    r2.x = static_cast<int>(gtk_adjustment_get_value(hadj));
    r2.y = static_cast<int>(gtk_adjustment_get_value(vadj));
    r2.width = static_cast<int>(gtk_adjustment_get_page_size(hadj));
    r2.height = static_cast<int>(gtk_adjustment_get_page_size(vadj));

    GdkRectangle r1;
    auto pos = p->getPixelPosition();
    r1.x = pos.x;
    r1.y = pos.y;
    r1.width = p->getDisplayWidth();
    r1.height = p->getDisplayHeight();

    GdkRectangle r3 = {0, 0, 0, 0};
    gdk_rectangle_intersect(&r1, &r2, &r3);

    if (r3.width == 0 && r3.height == 0) {
        return nullptr;
    }

    r3.x -= r1.x;
    r3.y -= r1.y;

    double zoom = xournal->view->getZoom();

    if (r3.x < 0 || r3.y < 0) {
        g_warning("XournalWidget:gtk_xournal_get_visible_area: intersection rectangle coordinates are negative which "
                  "should never happen");
    }

    return new xoj::util::Rectangle<double>(std::max(r3.x, 0) / zoom, std::max(r3.y, 0) / zoom, r3.width / zoom,
                                            r3.height / zoom);
}

auto gtk_xournal_get_layout(GtkWidget* widget) -> Layout* {
    g_return_val_if_fail(widget != nullptr, nullptr);
    g_return_val_if_fail(GTK_IS_XOURNAL(widget), nullptr);

    GtkXournal* xournal = GTK_XOURNAL(widget);
    return xournal->layout;
}

static void gtk_xournal_init(GtkXournal* xournal) { gtk_widget_set_focusable(GTK_WIDGET(xournal), true); }

static void gtk_xournal_measure(GtkWidget* widget, GtkOrientation orientation, int for_size, int* minimum, int* natural,
                                int* minimum_baseline, int* natural_baseline) {
    *minimum_baseline = *natural_baseline = -1;  // No baseline
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        *minimum = *natural = GTK_XOURNAL(widget)->layout->getMinimalPixelWidth();
    } else {
        *minimum = *natural = GTK_XOURNAL(widget)->layout->getMinimalPixelHeight();
    }
}

// TODO PUT THAT SOMEWHERE
// gtk_xournal_get_layout(widget)->recomputeCenteringPadding(allocation->width, allocation->height);


static void gtk_xournal_draw_shadow(GtkXournal* xournal, cairo_t* cr, int left, int top, int width, int height,
                                    bool selected) {
    if (selected) {
        Shadow::drawShadow(cr, left - 2, top - 2, width + 4, height + 4);

        Settings* settings = xournal->view->getControl()->getSettings();

        // Draw border
        Util::cairo_set_source_rgbi(cr, settings->getBorderColor());
        cairo_set_line_width(cr, 2.0);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);

        cairo_rectangle(cr, left - 1, top - 1, width + 2, height + 2);
        cairo_stroke(cr);
    } else {
        Shadow::drawShadow(cr, left, top, width, height);
    }
}

void gtk_xournal_repaint_area(GtkWidget* widget, int x1, int y1, int x2, int y2) {
    g_return_if_fail(widget != nullptr);
    g_return_if_fail(GTK_IS_XOURNAL(widget));

    if (x2 < 0 || y2 < 0 || x1 > gtk_widget_get_width(widget) || y1 > gtk_widget_get_height(widget)) {
        return;  // outside visible area
    }

    // TODO restore partial draws
    // gtk_widget_queue_draw_area(widget, x1, y1, x2 - x1, y2 - y1);
    gtk_widget_queue_draw(widget);
}

static void gtk_xournal_snapshot(GtkWidget* widget, GtkSnapshot* sn) {
    xoj_assert(widget != nullptr);
    xoj_assert(GTK_IS_XOURNAL(widget));


#ifdef DEBUG_DRAW_WIDGET
    {
        double x1 = NAN, x2 = NAN, y1 = NAN, y2 = NAN;
        cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
        printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n"
               "      DRAW  %d x %d + (%d ; %d)\n\n"
               "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$&&&&&&&&&&&&&&&&&&&&&&\n",
               round_cast<int>(x2 - x1), round_cast<int>(y2 - y1), round_cast<int>(x1), round_cast<int>(y1));
    }
#endif

    GtkXournal* xournal = GTK_XOURNAL(widget);

    GtkAdjustment* vadj = xournal->scrollHandling->getVertical();
    GtkAdjustment* hadj = xournal->scrollHandling->getHorizontal();

    auto rect = GRAPHENE_RECT_INIT_ZERO;
    rect.origin.x = static_cast<float>(gtk_adjustment_get_value(hadj));
    rect.origin.y = static_cast<float>(gtk_adjustment_get_value(vadj));
    rect.size.width = static_cast<float>(gtk_adjustment_get_page_size(hadj));
    rect.size.height = static_cast<float>(gtk_adjustment_get_page_size(vadj));

    xoj::util::CairoSPtr crsafe(gtk_snapshot_append_cairo(sn, &rect), xoj::util::adopt);
    cairo_t* cr = crsafe.get();

    Range clip;
    cairo_clip_extents(cr, &clip.minX, &clip.minY, &clip.maxX, &clip.maxY);

    // Draw background
    Settings* settings = xournal->view->getControl()->getSettings();
    Util::cairo_set_source_rgbi(cr, settings->getBackgroundColor());
    cairo_paint(cr);

    // Add a padding for the shadow of the pages
    clip.addPadding(10);

    const auto& views = xournal->view->getViewPages();
    // Store the pages to release the layout mutex ASAP
    std::vector<std::pair<size_t, xoj::util::Point<int>>> pages;
    xournal->layout->forEachEntriesIntersectingRange(
            clip, [&](size_t index, const Range&, xoj::util::Point<int> pos) { pages.emplace_back(index, pos); });

    for (auto [index, pos]: pages) {
        const auto& pv = views[index];
        int pw = pv->getDisplayWidth();
        int ph = pv->getDisplayHeight();

        gtk_xournal_draw_shadow(xournal, cr, pos.x, pos.y, pw, ph, pv->isSelected());

        cairo_save(cr);
        cairo_translate(cr, pos.x, pos.y);

        pv->paintPage(cr, nullptr);
        cairo_restore(cr);
    }

    if (xournal->selection) {
        cairo_save(cr);
        double zoom = xournal->view->getZoom();

        auto pos = xournal->selection->getView()->getPixelPosition();
        cairo_translate(cr, pos.x, pos.y);

        xournal->selection->paint(cr, zoom);
        cairo_restore(cr);
    }

    std::optional<Recolor> recolor = settings->getRecolorParameters().recolorizeMainView ?
                                             std::make_optional(settings->getRecolorParameters().recolor) :
                                             std::nullopt;

    if (recolor) {
        recolor->recolorCurrentCairoRegion(cr);
    }
}

static void gtk_xournal_dispose(GObject* object) {
    g_return_if_fail(object != nullptr);
    g_return_if_fail(GTK_IS_XOURNAL(object));
    GtkXournal* xournal = GTK_XOURNAL(object);

    delete xournal->selection;
    xournal->selection = nullptr;

    delete xournal->layout;
    xournal->layout = nullptr;

    delete xournal->input;
    xournal->input = nullptr;

    G_OBJECT_CLASS(gtk_xournal_parent_class)->dispose(object);
}
