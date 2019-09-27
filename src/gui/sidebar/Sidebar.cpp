#include "Sidebar.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "gui/GladeGui.h"
#include "indextree/SidebarIndexPage.h"
#include "model/Document.h"
#include "model/XojPage.h"
#include "previews/page/SidebarPreviewPages.h"
#include "previews/layer/SidebarPreviewLayers.h"

#include <config-features.h>

Sidebar::Sidebar(GladeGui* gui, Control* control)
 : toolbar(this, gui),
   control(control),
   gui(gui)
{
	this->tbSelectPage = GTK_TOOLBAR(gui->get("tbSelectSidebarPage"));
	this->buttonCloseSidebar = gui->get("buttonCloseSidebar");

	this->sidebar = gui->get("sidebarContents");

	this->initPages(sidebar, gui);

	registerListener(control);
}

void Sidebar::initPages(GtkWidget* sidebar, GladeGui* gui)
{
	addPage(new SidebarIndexPage(this->control, &this->toolbar));
	addPage(new SidebarPreviewPages(this->control, this->gui, &this->toolbar));
	addPage(new SidebarPreviewLayers(this->control, this->gui, &this->toolbar));

	// Init toolbar with icons

	int i = 0;
	for (AbstractSidebarPage* p : this->pages)
	{
		GtkToolItem* it = gtk_toggle_tool_button_new();
		p->tabButton = it;

		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it), gtk_image_new_from_icon_name(p->getIconName().c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR));
		g_signal_connect(it, "clicked", G_CALLBACK(&buttonClicked), new SidebarPageButton(this, i, p));
		gtk_tool_item_set_tooltip_text(it, p->getName().c_str());
		gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), p->getName().c_str());

		gtk_toolbar_insert(tbSelectPage, it, -1);

		// Add widget to sidebar
		gtk_box_pack_start(GTK_BOX(sidebar), p->getWidget(), true, true, 0);

		i++;
	}

	gtk_widget_show_all(GTK_WIDGET(this->tbSelectPage));

	updateEnableDisableButtons();
}

void Sidebar::buttonClicked(GtkToolButton* toolbutton, SidebarPageButton* buttonData)
{
	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton)))
	{
		if (buttonData->sidebar->visiblePage != buttonData->page->getWidget())
		{
			buttonData->sidebar->setSelectedPage(buttonData->index);
		}
	}
	else if (buttonData->sidebar->visiblePage == buttonData->page->getWidget())
	{
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton), true);
	}
}

void Sidebar::addPage(AbstractSidebarPage* page)
{
	this->pages.push_back(page);
}

Sidebar::~Sidebar()
{
	this->control = nullptr;

	for (AbstractSidebarPage* p : this->pages)
	{
		delete p;
	}
	this->pages.clear();

	this->sidebar = nullptr;
	this->currentPage = nullptr;
}

/**
 * Called when an action is performed
 */
void Sidebar::actionPerformed(SidebarActions action)
{
	if (!this->currentPage)
	{
		return;
	}

	this->currentPage->actionPerformed(action);
}

void Sidebar::selectPageNr(size_t page, size_t pdfPage)
{
	for (AbstractSidebarPage* p : this->pages)
	{
		p->selectPageNr(page, pdfPage);
	}
}

void Sidebar::setSelectedPage(size_t page)
{
	this->visiblePage = nullptr;
	this->currentPage = nullptr;

	size_t i = 0;
	for (AbstractSidebarPage* p : this->pages)
	{
		if (page == i)
		{
			gtk_widget_show(p->getWidget());
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(p->tabButton), true);
			this->visiblePage = p->getWidget();
			this->currentPage = p;
			p->enableSidebar();
		}
		else
		{
			p->disableSidebar();
			gtk_widget_hide(p->getWidget());
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(p->tabButton), false);
		}

		i++;
	}
}

void Sidebar::updateEnableDisableButtons()
{
	size_t i = 0;
	size_t selected = npos;

	for (AbstractSidebarPage* p : this->pages)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(p->tabButton), p->hasData());

		if (p->hasData() && selected == npos)
		{
			selected = i;
		}

		i++;
	}

	setSelectedPage(selected);
}

void Sidebar::setTmpDisabled(bool disabled)
{
	gtk_widget_set_sensitive(this->buttonCloseSidebar, !disabled);
	gtk_widget_set_sensitive(GTK_WIDGET(this->tbSelectPage), !disabled);

	for (AbstractSidebarPage* p : this->pages)
	{
		p->setTmpDisabled(disabled);
	}

	gdk_display_sync(gdk_display_get_default());
}

void Sidebar::saveSize()
{
	GtkAllocation alloc;
	gtk_widget_get_allocation(this->sidebar, &alloc);

	this->control->getSettings()->setSidebarWidth(alloc.width);
}

Control* Sidebar::getControl()
{
	return this->control;
}

void Sidebar::documentChanged(DocumentChangeType type)
{
	if (type == DOCUMENT_CHANGE_CLEARED || type == DOCUMENT_CHANGE_COMPLETE || type == DOCUMENT_CHANGE_PDF_BOOKMARKS)
	{
		updateEnableDisableButtons();
	}
}

SidebarPageButton::SidebarPageButton(Sidebar* sidebar, int index, AbstractSidebarPage* page)
{
	this->sidebar = sidebar;
	this->index = index;
	this->page = page;
}

