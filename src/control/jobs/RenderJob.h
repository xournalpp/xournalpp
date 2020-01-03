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

#include "Job.h"
#include "XournalType.h"

class Rectangle;
class XojPageView;

class RenderJob: public Job {
public:
    RenderJob(XojPageView* view);

protected:
    virtual ~RenderJob();

public:
    virtual JobType getType();

    void* getSource();

    void run();

private:
    /**
     * Repaint the widget in UI Thread
     */
    static void repaintWidget(GtkWidget* widget);

    void rerenderRectangle(Rectangle* rect);

private:
    XojPageView* view;
};
