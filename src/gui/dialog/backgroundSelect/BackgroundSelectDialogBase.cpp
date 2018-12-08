#include "BackgroundSelectDialogBase.h"

#include "BaseElementView.h"

#include <Util.h>


BackgroundSelectDialogBase::BackgroundSelectDialogBase(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings, string glade, string mainWnd)
 : GladeGui(gladeSearchPath, glade, mainWnd),
   settings(settings),
   scrollPreview(NULL),
   layoutContainer(NULL),
   doc(doc),
   confirmed(false),
   selected(-1),
   lastWidth(0)
{
	XOJ_INIT_TYPE(BackgroundSelectDialogBase);

	this->layoutContainer = gtk_layout_new(NULL, NULL);
	gtk_widget_show(this->layoutContainer);
	this->scrollPreview = get("scrollContents");
	gtk_container_add(GTK_CONTAINER(scrollPreview), layoutContainer);

	gtk_widget_set_events(this->layoutContainer, GDK_EXPOSURE_MASK);
	g_signal_connect(this->layoutContainer, "draw", G_CALLBACK(Util::paintBackgroundWhite), NULL);

	g_signal_connect(this->window, "size-allocate", G_CALLBACK(sizeAllocate), this);
	gtk_window_set_default_size(GTK_WINDOW(this->window), 800, 600);
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

void BackgroundSelectDialogBase::sizeAllocate(GtkWidget* widget, GtkRequisition* requisition, BackgroundSelectDialogBase* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, BackgroundSelectDialogBase);

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(dlg->scrollPreview, &alloc);
	if (dlg->lastWidth == alloc.width)
	{
		return;
	}
	dlg->lastWidth = alloc.width;
	dlg->layout();
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

		gtk_layout_move(GTK_LAYOUT(this->layoutContainer), p->getWidget(), x, y);

		height = MAX(height, p->getHeight());

		x += p->getWidth();
	}

	gtk_layout_set_size(GTK_LAYOUT(this->layoutContainer), width, y);
}

void BackgroundSelectDialogBase::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(BackgroundSelectDialogBase);

	for (BaseElementView* e : elements)
	{
		gtk_layout_put(GTK_LAYOUT(this->layoutContainer), e->getWidget(), 0, 0);
	}

	if (!elements.empty())
	{
		setSelected(0);
	}

	layout();

	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
}

void BackgroundSelectDialogBase::setSelected(int selected)
{
	XOJ_CHECK_TYPE(BackgroundSelectDialogBase);

	if (this->selected == selected)
	{
		return;
	}

	int lastSelected = this->selected;
	if (lastSelected >= 0 && lastSelected < (int)elements.size())
	{
		elements[lastSelected]->setSelected(false);
	}

	if (selected >= 0 && selected < (int)elements.size())
	{
		elements[selected]->setSelected(true);
		this->selected = selected;
	}
}


