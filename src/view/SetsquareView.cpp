#include "SetsquareView.h"

#include "gui/XournalView.h"
#include "model/Setsquare.h"

#include "DocumentView.h"

SetsquareView::SetsquareView(XojPageView* view, Setsquare* s): view(view), s(s) {}

void SetsquareView::paint(cairo_t* cr) {
    double zoom = view->getXournal()->getZoom();
    int dpiScaleFactor = view->getXournal()->getDpiScaleFactor();
    cairo_save(cr);
    cairo_scale(cr, zoom * dpiScaleFactor, zoom * dpiScaleFactor);
    this->s->paint(cr);
    cairo_restore(cr);
}

void SetsquareView::move(double x, double y) {
    double zoom = view->getXournal()->getZoom();
    s->move(x / zoom, y / zoom);
}

void SetsquareView::rotate(double da, double cx, double cy) { s->rotate(da); }

void SetsquareView::scale(double f) { s->scale(f); }

void SetsquareView::setView(XojPageView* view) { this->view = view; }

auto SetsquareView::getView() const -> XojPageView* { return view; }

auto SetsquareView::getPage() const -> PageRef { return view->getPage(); }
