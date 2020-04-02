#include "PresentationLaser.h"

#include <cmath>

static gboolean timeoutCallback(PresentationLaser* self) {
    self->step();
    return true;
}

PresentationLaser::PresentationLaser(GtkWidget* widget): widget(widget) {
    this->timeoutId = g_timeout_add(DELTA, reinterpret_cast<GSourceFunc>(timeoutCallback), this);
    g_object_ref(this->widget);
}

PresentationLaser::~PresentationLaser() {
    if (this->timeoutId != -1) {
        g_source_remove(this->timeoutId);
    }
    g_object_unref(this->widget);
}

void PresentationLaser::step() {
    const double distX = this->target.x - this->x;
    const double distY = this->target.y - this->y;

    if (distX == 0.0 && distY == 0.0) {
        return;
    }

    // The laser will move, so redraw the current position
    const Rectangle<double>&& oldBounds = this->getDrawBounds();
    gtk_widget_queue_draw_area(this->widget, oldBounds.x, oldBounds.y, oldBounds.width, oldBounds.height);

    const double angle = std::atan2(distY, distX);
    const double dx = std::abs(distX) < this->speed ? distX : speed * std::cos(angle);
    const double dy = std::abs(distY) < this->speed ? distY : speed * std::sin(angle);
    this->x += dx;
    this->y += dy;

    // Redraw the new location
    const Rectangle<double>&& newBounds = this->getDrawBounds();
    gtk_widget_queue_draw_area(this->widget, newBounds.x, newBounds.y, newBounds.width, newBounds.height);
}

void PresentationLaser::draw(cairo_t* cr) {
    cairo_save(cr);
    cairo_set_line_width(cr, LASER_RADIUS * 2);
    cairo_arc(cr, this->x, this->y, LASER_RADIUS, 0.0, 2 * M_PI);
    const double r = ((this->color >> 24) & 0xFF) / 255.0;
    const double g = ((this->color >> 16) & 0xFF) / 255.0;
    const double b = ((this->color >> 8) & 0xFF) / 255.0;
    const double a = (this->color & 0xFF) / 255.0;
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_fill(cr);
    cairo_restore(cr);
}

Rectangle<double> PresentationLaser::getDrawBounds() {
    // Overapproximate the draw bounds to avoid floating point inaccuracies.
    const double cb = LASER_RADIUS + 1.0;
    return {this->x - cb, this->y - cb, cb * 2, cb * 2};
}
