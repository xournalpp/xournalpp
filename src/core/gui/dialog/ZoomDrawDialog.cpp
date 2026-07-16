#include "ZoomDrawDialog.h"

#include <algorithm>  // for max

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/settings/Settings.h"
#include "gui/Builder.h"
#include "gui/MainWindow.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "undo/GroupUndoAction.h"
#include "undo/InsertUndoAction.h"
#include "undo/UndoRedoHandler.h"
#include "util/Color.h"
#include "util/Range.h"

ZoomDrawDialog::ZoomDrawDialog(GladeSearchpath* gladeSearchPath, Control* control, PageRef page, double centerX,
                               double centerY, int popupSidePx, double initialZoomFactor):
        control(control),
        page(std::move(page)),
        centerX(centerX),
        centerY(centerY),
        popupSidePx(popupSidePx),
        zoomFactor(initialZoomFactor) {
    Builder builder(gladeSearchPath, "zoomDrawDialog.glade");
    window.reset(GTK_WINDOW(builder.get("zoomDrawDialog")));

    canvas = builder.get("zoomCanvas");
    zoomFactorSpin = GTK_SPIN_BUTTON(builder.get("zoomFactorSpin"));
    btOk = GTK_BUTTON(builder.get("btOk"));
    btCancel = GTK_BUTTON(builder.get("btCancel"));

    gtk_widget_set_size_request(canvas, popupSidePx, popupSidePx);
    gtk_spin_button_set_value(zoomFactorSpin, zoomFactor);

    // Give the popup's PDF background rendering access to the real document's page cache,
    // otherwise it would only ever show a "missing PDF" placeholder.
    if (control->getWindow() && control->getWindow()->getXournal()) {
        docView.setPdfCache(control->getWindow()->getXournal()->getCache());
    }

    g_signal_connect(canvas, "draw", G_CALLBACK(+[](GtkWidget*, cairo_t* cr, gpointer data) -> gboolean {
                         static_cast<ZoomDrawDialog*>(data)->drawContents(cr);
                         return true;
                     }),
                     this);

    g_signal_connect(canvas, "button-press-event",
                     G_CALLBACK(+[](GtkWidget*, GdkEventButton* event, gpointer data) -> gboolean {
                         if (event->button == 1) {
                             static_cast<ZoomDrawDialog*>(data)->beginStroke(event->x, event->y);
                         }
                         return true;
                     }),
                     this);

    g_signal_connect(canvas, "motion-notify-event",
                     G_CALLBACK(+[](GtkWidget*, GdkEventMotion* event, gpointer data) -> gboolean {
                         static_cast<ZoomDrawDialog*>(data)->continueStroke(event->x, event->y);
                         return true;
                     }),
                     this);

    g_signal_connect(canvas, "button-release-event",
                     G_CALLBACK(+[](GtkWidget*, GdkEventButton* event, gpointer data) -> gboolean {
                         if (event->button == 1) {
                             static_cast<ZoomDrawDialog*>(data)->endStroke();
                         }
                         return true;
                     }),
                     this);

    g_signal_connect(zoomFactorSpin, "value-changed", G_CALLBACK(+[](GtkSpinButton* spin, gpointer data) {
                         auto* self = static_cast<ZoomDrawDialog*>(data);
                         self->zoomFactor = gtk_spin_button_get_value(spin);
                         self->control->getSettings()->setZoomDrawFactor(self->zoomFactor);
                         self->redrawCanvas();
                     }),
                     this);

    g_signal_connect(btCancel, "clicked", G_CALLBACK(+[](GtkButton*, gpointer data) {
                         static_cast<ZoomDrawDialog*>(data)->cancelAndClose();
                     }),
                     this);

    g_signal_connect(btOk, "clicked", G_CALLBACK(+[](GtkButton*, gpointer data) {
                         static_cast<ZoomDrawDialog*>(data)->applyAndClose();
                     }),
                     this);

    gtk_widget_grab_default(GTK_WIDGET(btOk));
}

ZoomDrawDialog::~ZoomDrawDialog() = default;

auto ZoomDrawDialog::getWindow() const -> GtkWindow* { return window.get(); }

auto ZoomDrawDialog::widgetToPage(double wx, double wy) const -> Point {
    double pageX = centerX + (wx - static_cast<double>(popupSidePx) / 2.0) / zoomFactor;
    double pageY = centerY + (wy - static_cast<double>(popupSidePx) / 2.0) / zoomFactor;
    return Point(pageX, pageY);
}

void ZoomDrawDialog::redrawCanvas() {
    if (canvas) {
        gtk_widget_queue_draw(canvas);
    }
}

void ZoomDrawDialog::drawContents(cairo_t* cr) {
    cairo_save(cr);

    // Fallback background, in case the page has transparent/unset regions at its edges
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    double half = static_cast<double>(popupSidePx) / 2.0;
    cairo_translate(cr, half, half);
    cairo_scale(cr, zoomFactor, zoomFactor);
    cairo_translate(cr, -centerX, -centerY);

    if (page) {
        // Render the page (background + all existing layers) exactly as it looks in the document.
        docView.drawPage(page, cr, /* dontRenderEditingStroke = */ true);
    }

    // Render the strokes drawn in this popup so far, directly in page-space: since the cairo
    // context above is already scaled by zoomFactor, using true page-unit widths/coordinates
    // here keeps them perfectly aligned with the underlying page content.
    auto drawStroke = [&](const Stroke& s) {
        const std::vector<Point>& pts = s.getPointVector();
        if (pts.size() < 2) {
            return;
        }
        Util::cairo_set_source_rgbi(cr, s.getColor());
        cairo_set_line_width(cr, std::max(s.getWidth(), 0.01));
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
        cairo_move_to(cr, pts.front().x, pts.front().y);
        for (size_t i = 1; i < pts.size(); i++) { cairo_line_to(cr, pts[i].x, pts[i].y); }
        cairo_stroke(cr);
    };

    for (auto const& s: strokes) { drawStroke(*s); }
    if (currentStroke) {
        drawStroke(*currentStroke);
    }

    cairo_restore(cr);

    // Draw a small reference marker at the exact center of the canvas (i.e. at the originally
    // clicked point P), in plain widget-pixel space, so the user can see what P was aligned to.
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.9, 0.1, 0.1, 0.6);
    cairo_set_line_width(cr, 1.0);
    double markerSize = 8.0;
    cairo_move_to(cr, half - markerSize, half);
    cairo_line_to(cr, half + markerSize, half);
    cairo_move_to(cr, half, half - markerSize);
    cairo_line_to(cr, half, half + markerSize);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void ZoomDrawDialog::beginStroke(double wx, double wy) {
    dragging = true;

    currentStroke = std::make_unique<Stroke>();
    ToolHandler* h = control->getToolHandler();
    currentStroke->setColor(h->getColor());
    currentStroke->setWidth(h->getThickness());
    currentStroke->setToolType(StrokeTool::PEN);

    Point p = widgetToPage(wx, wy);
    currentStroke->addPoint(p);
}

void ZoomDrawDialog::continueStroke(double wx, double wy) {
    if (!dragging || !currentStroke) {
        return;
    }
    Point p = widgetToPage(wx, wy);
    currentStroke->addPoint(p);
    redrawCanvas();
}

void ZoomDrawDialog::endStroke() {
    if (!dragging) {
        return;
    }
    dragging = false;

    if (currentStroke) {
        currentStroke->freeUnusedPointItems();
        // Discard degenerate strokes produced by a plain click without dragging.
        if (currentStroke->getPointCount() > 1) {
            strokes.push_back(std::move(currentStroke));
        }
        currentStroke.reset();
        redrawCanvas();
    }
}

void ZoomDrawDialog::applyAndClose() {
    endStroke();  // Just in case the button was released outside the canvas.

    if (!strokes.empty() && page) {
        Layer* layer = page->getSelectedLayer();
        Document* doc = control->getDocument();
        UndoRedoHandler* undo = control->getUndoRedoHandler();

        auto group = std::make_unique<GroupUndoAction>();
        Range range;

        doc->lock();
        for (auto& s: strokes) {
            Stroke* raw = s.get();
            range = range.unite(Range(raw->boundingRect()));

            // The InsertUndoAction must be created *before* the element is actually moved into
            // the layer, mirroring the order used by the regular pen tool (StrokeHandler).
            group->addAction(std::make_unique<InsertUndoAction>(page, layer, raw));
            layer->addElement(std::move(s));
        }
        doc->unlock();
        strokes.clear();

        undo->addUndoAction(std::move(group));

        page->fireRangeChanged(range);
    }

    gtk_window_close(window.get());
}

void ZoomDrawDialog::cancelAndClose() {
    // Discard everything: the strokes were never added to the document, so simply dropping
    // them (via the destructor) is enough.
    dragging = false;
    currentStroke.reset();
    strokes.clear();

    gtk_window_close(window.get());
}
