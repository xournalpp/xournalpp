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

#include "Job.h"

#include <XournalType.h>

#include <gtk/gtk.h>

class SidebarPreviewBaseEntry;
class Document;

/**
 * @brief A Job which renders a SidebarPreviewPage
 */
class PreviewJob : public Job
{
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
	void drawBorder();
	void finishPaint();
	void drawBackgroundPdf(Document* doc);
	void drawPage(int layer);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * Graphics buffer
	 */
	cairo_surface_t* crBuffer = NULL;

	/**
	 * Graphics drawing
	 */
	cairo_t* cr2 = NULL;

	/**
	 * Zoom factor
	 */
	double zoom = 0;

	/**
	 * Sidebar preview
	 */
	SidebarPreviewBaseEntry* sidebarPreview = NULL;
};
