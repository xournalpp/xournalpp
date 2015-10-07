#include "SidebarPreviewBase.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "SidebarLayout.h"
#include "SidebarPreviewBaseEntry.h"
#include "SidebarToolbar.h"

SidebarPreviewBase::SidebarPreviewBase(Control* control) : AbstractSidebarPage(control)
{
	XOJ_INIT_TYPE(SidebarPreviewBase);

	this->backgroundInitialized = false;

	this->layoutmanager = new SidebarLayout();
	this->toolbar = new SidebarToolbar(control);

	this->zoom = 0.15;

	this->selectedEntry = -1;

	this->cache = new PdfCache(control->getSettings()->getPdfPageCacheSize());

	this->iconViewPreview = gtk_layout_new(NULL, NULL);
	g_object_ref(this->iconViewPreview);

	this->scrollPreview = gtk_scrolled_window_new(NULL, NULL);
	g_object_ref(this->scrollPreview);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(this->scrollPreview), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(this->scrollPreview), GTK_SHADOW_IN);

	gtk_container_add(GTK_CONTAINER(this->scrollPreview), this->iconViewPreview);
	gtk_widget_show(this->scrollPreview);

	gtk_widget_show(this->iconViewPreview);

	registerListener(this->control);

	g_signal_connect(this->scrollPreview, "size-allocate", G_CALLBACK(sizeChanged), this);

	this->table = GTK_TABLE(gtk_table_new(2, 1, false));
	g_object_ref(this->table);

	gtk_table_attach(this->table, this->scrollPreview, 0, 1, 0, 1,
					 (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), 0, 0);
	gtk_table_attach(this->table, this->toolbar->getWidget(), 0, 1, 1, 2,
					 (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
}

SidebarPreviewBase::~SidebarPreviewBase()
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);

	gtk_widget_destroy(this->iconViewPreview);
	this->iconViewPreview = NULL;

	delete this->cache;
	this->cache = NULL;

	delete this->layoutmanager;
	this->layoutmanager = NULL;

	delete this->toolbar;
	this->toolbar = NULL;

	g_object_unref(this->table);

	for (SidebarPreviewBaseEntry* p : this->previews)
	{
		delete p;
	}
	this->previews.clear();

	XOJ_RELEASE_TYPE(SidebarPreviewBase);
}

void SidebarPreviewBase::sizeChanged(GtkWidget* widget, GtkAllocation* allocation, SidebarPreviewBase* sidebar)
{
	XOJ_CHECK_TYPE_OBJ(sidebar, SidebarPreviewBase);

	static int lastWidth = -1;

	if (lastWidth == -1)
	{
		lastWidth = allocation->width;
	}

	if (ABS(lastWidth - allocation->width) > 20)
	{
		sidebar->layout();
		lastWidth = allocation->width;
	}
}

void SidebarPreviewBase::setBackgroundWhite()
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);

	if (this->backgroundInitialized)
	{
		return;
	}
	this->backgroundInitialized = true;

	gdk_window_set_background(GTK_LAYOUT(this->iconViewPreview)->bin_window, &this->iconViewPreview->style->white);
}

double SidebarPreviewBase::getZoom()
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);

	return this->zoom;
}

PdfCache* SidebarPreviewBase::getCache()
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);

	return this->cache;
}

void SidebarPreviewBase::layout()
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);

	this->layoutmanager->layout(this);
}

bool SidebarPreviewBase::hasData()
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);

	return true;
}

GtkWidget* SidebarPreviewBase::getWidget()
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);

	return GTK_WIDGET(this->table);
}

void SidebarPreviewBase::documentChanged(DocumentChangeType type)
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);

	if (type == DOCUMENT_CHANGE_COMPLETE || type == DOCUMENT_CHANGE_CLEARED)
	{
		updatePreviews();
	}
}

bool SidebarPreviewBase::scrollToPreview(SidebarPreviewBase* sidebar)
{
	XOJ_CHECK_TYPE_OBJ(sidebar, SidebarPreviewBase);

	MainWindow* win = sidebar->control->getWindow();
	if (win)
	{
		GtkWidget* w = win->get("sidebarContents");
		if (!gtk_widget_get_visible(w))
		{
			return false;
		}
	}

	if (sidebar->selectedEntry != size_t_npos && sidebar->selectedEntry < sidebar->previews.size())
	{
		gdk_threads_enter();
		SidebarPreviewBaseEntry* p = sidebar->previews[sidebar->selectedEntry];

		// scroll to preview
		GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(sidebar->scrollPreview));
		GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sidebar->scrollPreview));
		GtkWidget* widget = p->getWidget();

		int x = widget->allocation.x;
		int y = widget->allocation.y;
		gdk_threads_leave();

		if (x == -1)
		{
			g_idle_add((GSourceFunc) scrollToPreview, sidebar);
			return false;
		}

		gdk_threads_enter();
		gtk_adjustment_clamp_page(vadj, y, y + widget->allocation.height);
		gtk_adjustment_clamp_page(hadj, x, x + widget->allocation.width);
		gdk_threads_leave();
	}
	return false;
}

void SidebarPreviewBase::pageDeleted(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);
}

void SidebarPreviewBase::pageInserted(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewBase);
}

