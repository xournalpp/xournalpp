/*
 * Xournal++
 *
 * A job which handles preview repaint
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

#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"

#include "Job.h"
#include "XournalType.h"

class SidebarPreviewBaseEntry;
class Document;

/**
 * @brief A Job which renders a SidebarPreviewPage
 */
class PreviewJob: public Job {
public:
    PreviewJob(SidebarPreviewBaseEntry* sidebar);

protected:
    virtual ~PreviewJob();

public:
    virtual void* getSource();

    virtual void run();

    virtual JobType getType();

private:
    void initGraphics();
    void clipToPage();
    void drawBorder();
    void finishPaint();
    void drawBackgroundPdf(Document* doc);
    void drawPage();

private:
    /**
     * Graphics buffer
     */
    cairo_surface_t* crBuffer = nullptr;

    /**
     * Graphics drawing
     */
    cairo_t* cr2 = nullptr;

    /**
     * Zoom factor
     */
    double zoom = 0;

    /**
     * ID for sidebar's onDestroy listener (for cases where the sidebar
     * is destroyed before this).
     */
    SidebarPreviewBaseEntry::OnDestroyListenerID previewDestroyListenerID{};

    /**
     * Sidebar preview
     */
    SidebarPreviewBaseEntry* sidebarPreview = nullptr;
};
