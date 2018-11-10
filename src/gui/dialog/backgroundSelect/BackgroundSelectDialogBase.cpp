#include "BackgroundSelectDialogBase.h"

#include "BaseElementView.h"

BackgroundSelectDialogBase::BackgroundSelectDialogBase(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings, string glade, string mainWnd)
 : GladeGui(gladeSearchPath, glade, mainWnd),
   settings(settings)
{
	XOJ_INIT_TYPE(BackgroundSelectDialogBase);

	this->widget = gtk_layout_new(NULL, NULL);
	gtk_widget_show(this->widget);
	this->scrollPreview = get("scrollContens");
	gtk_container_add(GTK_CONTAINER(scrollPreview), widget);

//	gdk_window_set_background(gtk_layout_get_bin_window(GTK_LAYOUT(this->widget)), &gtk_widget_get_style(widget)->white);

	// TODO Resize
//	g_signal_connect(this->window, "size-allocate", G_CALLBACK(sizeAllocate), this);

	gtk_widget_set_size_request(this->window, 800, 600);
}

BackgroundSelectDialogBase::~BackgroundSelectDialogBase()
{
	XOJ_CHECK_TYPE(BackgroundSelectDialogBase);

	for (BaseElementView* e : elements)
	{
		delete e;
	}
	elements.clear();

	XOJ_RELEASE_TYPE(BackgroundSelectDialogBase);
}

Settings* BackgroundSelectDialogBase::getSettings()
{
	XOJ_CHECK_TYPE(BackgroundSelectDialogBase);

	return this->settings;
}

void BackgroundSelectDialogBase::layout()
{
	XOJ_CHECK_TYPE(BackgroundSelectDialogBase);

	double x = 0;
	double y = 0;
	double height = 0;
	double width = 0;

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(this->scrollPreview, &alloc);

	for (BaseElementView* p : this->elements)
	{
		if (!gtk_widget_get_visible(p->getWidget()))
		{
			continue;
		}

		if (x + p->getWidth() > alloc.width)
		{
			width = MAX(width, x);
			y += height;
			x = 0;
			height = 0;
		}

		gtk_layout_move(GTK_LAYOUT(this->widget), p->getWidget(), x, y);

		height = MAX(height, p->getHeight());

		x += p->getWidth();
	}

	gtk_layout_set_size(GTK_LAYOUT(this->widget), width, y);
}

void BackgroundSelectDialogBase::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(BackgroundSelectDialogBase);

	for (BaseElementView* e : elements)
	{
//		GtkWidget* w = gtk_label_new("55555555");
//		gtk_layout_put(GTK_LAYOUT(this->widget), w, 0, 0);
//		gtk_widget_set_visible(w, true);
		gtk_layout_put(GTK_LAYOUT(this->widget), e->getWidget(), 0, 0);
	}


	layout();

	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
}

