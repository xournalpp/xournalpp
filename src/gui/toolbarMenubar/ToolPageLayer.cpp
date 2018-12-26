#include "ToolPageLayer.h"

#include "control/LayerController.h"
#include "gui/GladeGui.h"
#include "gui/widgets/PopupMenuButton.h"

#include <config.h>
#include <i18n.h>

ToolPageLayer::ToolPageLayer(LayerController* lc, GladeGui* gui, ActionHandler* handler, string id, ActionType type)
 : AbstractToolItem(id, handler, type, NULL),
   lc(lc),
   gui(gui),
   menu(gtk_menu_new()),
   menuY(0)
{
	XOJ_INIT_TYPE(ToolPageLayer);

	this->layerLabel = gtk_label_new("Test123");
	this->layerButton = gtk_button_new_with_label("⌄");

	PangoAttrList* attrs = pango_attr_list_new();
	pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
	gtk_label_set_attributes(GTK_LABEL(this->layerLabel), attrs);

	popupMenuButton = new PopupMenuButton(this->layerButton, menu);

	updateMenu();
}

ToolPageLayer::~ToolPageLayer()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	delete popupMenuButton;
	popupMenuButton = NULL;

	XOJ_RELEASE_TYPE(ToolPageLayer);
}

const int MENU_WIDTH = 3;

void ToolPageLayer::createSeparator()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	gtk_menu_attach(GTK_MENU(menu), gtk_separator_menu_item_new(), 0, MENU_WIDTH, menuY, menuY + 1);
	menuY++;
}

GtkWidget* ToolPageLayer::createSpecialMenuEntry(string name)
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	GtkWidget* it = gtk_menu_item_new();
	GtkWidget* lb = gtk_label_new(name.c_str());
	gtk_widget_set_halign(lb, GTK_ALIGN_START);

	PangoAttrList* attrs = pango_attr_list_new();
	pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
	gtk_label_set_attributes(GTK_LABEL(lb), attrs);

	gtk_container_add(GTK_CONTAINER(it), lb);
	gtk_menu_attach(GTK_MENU(menu), it, 0, MENU_WIDTH, menuY, menuY + 1);
	menuY++;

	return it;
}

/**
 * Add special button to the top of the menu
 */
void ToolPageLayer::addSpecialButtonTop()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	GtkWidget* itShowAll = createSpecialMenuEntry(_("Show all"));
	GtkWidget* itHideAll = createSpecialMenuEntry(_("Hide all"));
	createSeparator();

	g_signal_connect(itShowAll, "activate", G_CALLBACK(
		+[](GtkWidget* menu, ToolPageLayer* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, ToolPageLayer);
			self->lc->showAllLayer();
		}), this);


	g_signal_connect(itHideAll, "activate", G_CALLBACK(
		+[](GtkWidget* menu, ToolPageLayer* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, ToolPageLayer);
			self->lc->hideAllLayer();
		}), this);
}

/**
 * Rebuild the Menu
 */
void ToolPageLayer::updateMenu()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	// Remove all items from Menu
	gtk_container_foreach(GTK_CONTAINER(menu), (GtkCallback) gtk_widget_destroy, NULL);
	menuY = 0;

	addSpecialButtonTop();

	for (int layer = 10; layer > 0; layer--)
	{
		string text = FS(_F("Layer {1}") % (layer + 1));

		GtkWidget* itLayer = gtk_check_menu_item_new_with_label(text.c_str());
		gtk_menu_attach(GTK_MENU(menu), itLayer, 0, 2, menuY, menuY + 1);

		GtkWidget* itShow = gtk_check_menu_item_new_with_label(_("show"));
		gtk_menu_attach(GTK_MENU(menu), itShow, 2, 3, menuY, menuY + 1);
		gtk_widget_set_hexpand(itShow, false);

		menuY++;
	}

	gtk_menu_attach(GTK_MENU(menu), gtk_separator_menu_item_new(), 0, MENU_WIDTH, menuY, menuY + 1);
	menuY++;

	GtkWidget* itBackground = gtk_check_menu_item_new_with_label(_("Background"));
	gtk_menu_attach(GTK_MENU(menu), itBackground, 0, 2, menuY, menuY + 1);

	GtkWidget* itShowBackground = gtk_check_menu_item_new_with_label(_("show"));
	gtk_menu_attach(GTK_MENU(menu), itShowBackground, 2, 3, menuY, menuY + 1);
	gtk_widget_set_hexpand(itShowBackground, false);
	menuY++;

	gtk_widget_show_all(menu);
}

string ToolPageLayer::getToolDisplayName()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	return _("Layer selection");
}

GtkWidget* ToolPageLayer::getNewToolIcon()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	return gui->loadIcon("layers.svg");
}

GtkToolItem* ToolPageLayer::newItem()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	GtkToolItem* it = gtk_tool_item_new();

	GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_("Layer")), false, false, 7);

	gtk_box_pack_start(GTK_BOX(hbox), this->layerLabel, false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), this->layerButton, false, false, 0);

	gtk_container_add(GTK_CONTAINER(it), hbox);

	return it;
}
