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

#include "util/Rectangle.h"

#include "Job.h"


class XojPageView;

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
    /**
     * Repaint the widget in UI Thread
     */
    static void repaintWidget(GtkWidget* widget);

    void rerenderRectangle(xoj::util::Rectangle<double> const& rect);

private:
    XojPageView* view;
};
