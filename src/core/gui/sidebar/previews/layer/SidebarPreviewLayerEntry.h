/*
 * Xournal++
 *
 * A Sidebar preview widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include <gtk/gtk.h>  // for GtkWi...

#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"  // for Previ...
#include "model/Layer.h"                                        // for Layer
#include "model/PageRef.h"                                      // for PageRef
#include "util/raii/GObjectSPtr.h"

class SidebarPreviewLayers;

class SidebarPreviewLayerEntry: public SidebarPreviewBaseEntry {
public:
    SidebarPreviewLayerEntry(SidebarPreviewLayers* sidebar, const PageRef& page, Layer::Index layerId,
                             const std::string& layerName, bool stacked);
    ~SidebarPreviewLayerEntry() override;

public:
    /**
     * @return What should be rendered
     * @override
     */
    PreviewRenderType getRenderType() const override;

    /**
     * @return The layer to be rendered
     */
    Layer::Index getLayer() const;

    GtkWidget* getWidget() const override;

    /**
     * Set the value of the visible checkbox
     */
    void setVisibleCheckbox(bool enabled);

protected:
    void mouseButtonPressCallback() override;
    void checkboxToggled();

    SidebarPreviewLayers* sidebar;

private:
    /// Layer to render
    Layer::Index layerId;

    /// Container box for the preview and the button
    xoj::util::WidgetSPtr box;

    /// Visibility checkbox
    GtkWidget* cbVisible = nullptr;
    gulong callbackId = 0;

    /// render as stacked
    bool stacked = false;

    friend class PreviewJob;
};
