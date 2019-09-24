#include "ToolPageSpinner.h"

#include "gui/GladeGui.h"
#include "gui/widgets/SpinPageAdapter.h"

#include <config.h>
#include <i18n.h>

ToolPageSpinner::ToolPageSpinner(GladeGui* gui, ActionHandler* handler, string id, ActionType type)
 : AbstractToolItem(id, handler, type, nullptr)
{
	this->gui = gui;
	this->pageSpinner = new SpinPageAdapter();
}

ToolPageSpinner::~ToolPageSpinner()
{
	delete this->pageSpinner;
	this->pageSpinner = nullptr;
}

SpinPageAdapter* ToolPageSpinner::getPageSpinner()
{
	return pageSpinner;
}

void ToolPageSpinner::setText(string text)
{
	if (lbPageNo)
	{
		gtk_label_set_text(GTK_LABEL(lbPageNo), text.c_str());
	}
}

string ToolPageSpinner::getToolDisplayName()
{
	return _("Page number");
}

GtkWidget* ToolPageSpinner::getNewToolIcon()
{
	return gtk_image_new_from_icon_name("pageSpinner", GTK_ICON_SIZE_SMALL_TOOLBAR);
}

GtkToolItem* ToolPageSpinner::newItem()
{
	GtkToolItem* it = gtk_tool_item_new();

	GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);

	GtkWidget* pageLabel = gtk_label_new(_("Page"));
	GtkWidget* spinner = this->pageSpinner->getWidget();

	gtk_widget_set_valign(pageLabel, GTK_ALIGN_BASELINE);
	gtk_box_pack_start(GTK_BOX(hbox), pageLabel, false, false, 7);

	gtk_widget_set_valign(spinner, GTK_ALIGN_BASELINE);
	gtk_box_pack_start(GTK_BOX(hbox), spinner, false, false, 0);

	this->lbPageNo = gtk_label_new("");
	gtk_widget_set_valign(this->lbPageNo, GTK_ALIGN_BASELINE);
	gtk_box_pack_start(GTK_BOX(hbox), this->lbPageNo, false, false, 7);

	gtk_container_add(GTK_CONTAINER(it), hbox);

	return it;
}
