#include "SidebarPreviewBase.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "SidebarLayout.h"
#include "SidebarPreviewBaseEntry.h"


SidebarPreviewBase::SidebarPreviewBase(Control* control, GladeGui* gui, SidebarToolbar* toolbar)
 : AbstractSidebarPage(control, toolbar)
{
	this->layoutmanager = new SidebarLayout();

	this->cache = new PdfCache(control->getSettings()->getPdfPageCacheSize());

	this->iconViewPreview = gtk_layout_new(nullptr, nullptr);
	g_object_ref(this->iconViewPreview);

	this->scrollPreview = gtk_scrolled_window_new(nullptr, nullptr);
	g_object_ref(this->scrollPreview);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(this->scrollPreview), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(this->scrollPreview), GTK_SHADOW_IN);

	gtk_container_add(GTK_CONTAINER(this->scrollPreview), this->iconViewPreview);
	gtk_widget_show(this->scrollPreview);

	gtk_widget_show(this->iconViewPreview);

	registerListener(this->control);

	g_signal_connect(this->scrollPreview, "size-allocate", G_CALLBACK(sizeChanged), this);

	gtk_widget_show_all(this->scrollPreview);

	g_signal_connect(this->iconViewPreview, "draw", G_CALLBACK(Util::paintBackgroundWhite), nullptr);
}

SidebarPreviewBase::~SidebarPreviewBase()
{
	gtk_widget_destroy(this->iconViewPreview);
	this->iconViewPreview = nullptr;

	delete this->cache;
	this->cache = nullptr;

	delete this->layoutmanager;
	this->layoutmanager = nullptr;

	this->scrollPreview = nullptr;

	for (SidebarPreviewBaseEntry* p : this->previews)
	{
		delete p;
	}
	this->previews.clear();
}

void SidebarPreviewBase::enableSidebar()
{
	enabled = true;
}

void SidebarPreviewBase::disableSidebar()
{
	enabled = false;
}

void SidebarPreviewBase::sizeChanged(GtkWidget* widget, GtkAllocation* allocation, SidebarPreviewBase* sidebar)
{
	static int lastWidth = -1;

	if (lastWidth == -1)
	{
		lastWidth = allocation->width;
	}

	if (std::abs(lastWidth - allocation->width) > 20)
	{
		sidebar->layout();
		lastWidth = allocation->width;
	}
}

double SidebarPreviewBase::getZoom()
{
	return this->zoom;
}

PdfCache* SidebarPreviewBase::getCache()
{
	return this->cache;
}

void SidebarPreviewBase::layout()
{
	this->layoutmanager->layout(this);
}

bool SidebarPreviewBase::hasData()
{
	return true;
}

GtkWidget* SidebarPreviewBase::getWidget()
{
	return this->scrollPreview;
}

void SidebarPreviewBase::documentChanged(DocumentChangeType type)
{
	if (type == DOCUMENT_CHANGE_COMPLETE || type == DOCUMENT_CHANGE_CLEARED)
	{
		updatePreviews();
	}
}

bool SidebarPreviewBase::scrollToPreview(SidebarPreviewBase* sidebar)
{
	if (!sidebar->enabled)
	{
		return false;
	}

	MainWindow* win = sidebar->control->getWindow();
	if (win == nullptr)
	{
		return false;
	}

	GtkWidget* w = win->get("sidebar");
	if (!gtk_widget_get_visible(w))
	{
		return false;
	}

	if (sidebar->selectedEntry != npos && sidebar->selectedEntry < sidebar->previews.size())
	{
		SidebarPreviewBaseEntry* p = sidebar->previews[sidebar->selectedEntry];

		// scroll to preview
		GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(sidebar->scrollPreview));
		GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sidebar->scrollPreview));
		GtkWidget* widget = p->getWidget();

		GtkAllocation allocation;
		gtk_widget_get_allocation(widget, &allocation);
		int x = allocation.x;
		int y = allocation.y;

		if (x == -1)
		{
			g_idle_add((GSourceFunc) scrollToPreview, sidebar);
			return false;
		}

		gtk_adjustment_clamp_page(vadj, y, y + allocation.height);
		gtk_adjustment_clamp_page(hadj, x, x + allocation.width);
	}
	return false;
}

void SidebarPreviewBase::pageDeleted(int page)
{
}

void SidebarPreviewBase::pageInserted(int page)
{
}

