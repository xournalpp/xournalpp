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

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "util/Point.h"
#include "util/Rectangle.h"

#include "Job.h"


class XojPageView;

class RenderJob: public Job {
public:
    RenderJob(XojPageView* view);

protected:
    virtual ~RenderJob() = default;

public:
    virtual JobType getType();

    void* getSource();

    void run();

private:
    /**
     * Repaint the widget in UI Thread
     */
    static void repaintWidget(GtkWidget* widget);

    void rerenderTile(utl::Point<double> const& view_offset, utl::Point<double> const& document_offset) const;

    void rerenderRectangle(Rectangle<double> const& rect) const;

private:
    XojPageView* view;
};
