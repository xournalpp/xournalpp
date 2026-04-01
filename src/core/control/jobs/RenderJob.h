/*
 * Xournal++
 *
 * A job which redraws a page or a page region
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>    // for cairo_surface_t
#include <gtk/gtk.h>  // for GtkWidget

#include "Job.h"  // for Job, JobType

class XojPageView;
namespace xoj::util {
template <class T>
class Rectangle;
}  // namespace xoj::util

class RenderJob: public Job {
public:
    RenderJob(XojPageView* view);

protected:
    ~RenderJob() override = default;

public:
    JobType getType() override;

    void* getSource() override;

    void run() override;

private:
    void repaintPage() const;

    void repaintPageArea(double x1, double y1, double x2, double y2) const;

    void rerenderRectangle(xoj::util::Rectangle<double> const& rect);

    void renderToBuffer(cairo_t* cr) const;

private:
    XojPageView* view;
};
