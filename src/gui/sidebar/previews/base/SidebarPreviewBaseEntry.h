/*
 * Xournal++
 *
 * A preview entry in a sidebar
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>
#include <map>
#include <string>

#include <gtk/gtk.h>

#include "model/PageRef.h"

#include "Util.h"
#include "XournalType.h"

class SidebarPreviewBase;

typedef enum {
    /**
     * Render the whole page
     */
    RENDER_TYPE_PAGE_PREVIEW = 1,

    /**
     * Render only a layer
     */
    RENDER_TYPE_PAGE_LAYER,

    /**
     * Render the stack up to a layer
     */
    RENDER_TYPE_PAGE_LAYERSTACK
} PreviewRenderType;


class SidebarPreviewBaseEntry {
public:
    SidebarPreviewBaseEntry(SidebarPreviewBase* sidebar, const PageRef& page);
    virtual ~SidebarPreviewBaseEntry();

public:
    virtual GtkWidget* getWidget();
    virtual int getWidth();
    virtual int getHeight();

    virtual void setSelected(bool selected);

    virtual void repaint();
    virtual void updateSize();

    /**
     * @return What should be rendered
     */
    virtual PreviewRenderType getRenderType() = 0;

private:
    static gboolean drawCallback(GtkWidget* widget, cairo_t* cr, SidebarPreviewBaseEntry* preview);

protected:
    using OnDestroyListener = std::function<void(void)>;
    using OnDestroyListenerID = int;

    virtual void mouseButtonPressCallback() = 0;

    virtual int getWidgetWidth();
    virtual int getWidgetHeight();

    virtual void drawLoadingPage();
    virtual void paint(cairo_t* cr);

    /**
     * @brief Adds a listener that will be called once -- when this is destroyed.
     */
    OnDestroyListenerID addOnDestroyListener(OnDestroyListener&& listener);
    void removeOnDestroyListener(const OnDestroyListenerID& listenerID);

private:
protected:
    /**
     * If this page is currently selected
     */
    bool selected = false;

    /**
     * The sidebar which displays the previews
     */
    SidebarPreviewBase* sidebar;

    /**
     * The page which is representated
     */
    PageRef page;

    /**
     * Mutex
     */
    GMutex drawingMutex{};

    /**
     * The Widget which is used for drawing
     */
    GtkWidget* widget;

    /**
     * Buffer because of performance reasons
     */
    cairo_surface_t* crBuffer = nullptr;

    /**
     * onDestroy listeners
     */
    std::map<OnDestroyListenerID, OnDestroyListener> onDestroyListeners;
    OnDestroyListenerID nextOnDestroyListenerID{0};

    friend class PreviewJob;
};
