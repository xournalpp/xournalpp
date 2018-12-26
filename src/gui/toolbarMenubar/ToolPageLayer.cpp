#include "ToolPageLayer.h"

#include "control/layer/LayerController.h"
#include "gui/GladeGui.h"
#include "gui/widgets/PopupMenuButton.h"

#include <config.h>
#include <i18n.h>

ToolPageLayer::ToolPageLayer(LayerController* lc, GladeGui* gui, ActionHandler* handler, string id, ActionType type)
 : AbstractToolItem(id, handler, type, NULL),
   lc(lc),
   gui(gui),
   menu(gtk_menu_new()),
   menuY(0),
   inMenuUpdate(false)
{
	XOJ_INIT_TYPE(ToolPageLayer);

	this->layerLabel = gtk_label_new(_("Loading..."));
	this->layerButton = gtk_button_new_with_label("⌄");

	PangoAttrList* attrs = pango_attr_list_new();
	pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
	gtk_label_set_attributes(GTK_LABEL(this->layerLabel), attrs);

	popupMenuButton = new PopupMenuButton(this->layerButton, menu);

	LayerCtrlListener::registerListener(lc);
}

ToolPageLayer::~ToolPageLayer()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	delete popupMenuButton;
	popupMenuButton = NULL;

	XOJ_RELEASE_TYPE(ToolPageLayer);
}

void ToolPageLayer::rebuildLayerMenu()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	updateMenu();
}

void ToolPageLayer::layerVisibilityChanged()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	updateLayerData();
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

void ToolPageLayer::selectLayer(int layerId)
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	lc->switchToLay(layerId);
}

void ToolPageLayer::layerMenuClicked(GtkWidget* menu)
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	int layerId = -1;

	for (auto& kv : layerItems)
	{
		if (kv.second == menu)
		{
			layerId = kv.first;
			break;
		}
	}

	if (layerId < 0)
	{
		g_warning("Invalid Layer Menu selected - not handled");
		return;
	}


	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu)))
	{
		if (layerId == lc->getCurrentLayerId())
		{
			// This is the current layer, don't allow to deselect it
			inMenuUpdate = true;
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu), true);
			inMenuUpdate = false;
		}
		return;
	}

	selectLayer(layerId);
}

void ToolPageLayer::createLayerMenuItem(string text, int layerId)
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	GtkWidget* itLayer = gtk_check_menu_item_new_with_label(text.c_str());
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(itLayer), true);
	gtk_menu_attach(GTK_MENU(menu), itLayer, 0, 2, menuY, menuY + 1);

	g_signal_connect(itLayer, "activate", G_CALLBACK(
		+[](GtkWidget* menu, ToolPageLayer* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, ToolPageLayer);

			self->layerMenuClicked(menu);
		}), this);

	layerItems[layerId] = itLayer;
}

/**
 * Rebuild the Menu
 */
void ToolPageLayer::updateMenu()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	// Remove all items from Menu (does sometimes not work)
	// gtk_container_foreach(GTK_CONTAINER(menu), (GtkCallback) gtk_widget_destroy, NULL);

	// Create a new menu on refresh
	menu = gtk_menu_new();
	popupMenuButton->setMenu(menu);
	layerItems.clear();
	showLayerItems.clear();

	menuY = 0;

	addSpecialButtonTop();

	int layer = lc->getLayerCount();
	for (; layer > 0; layer--)
	{
		createLayerMenuItem(FS(_F("Layer {1}") % layer), layer);

		GtkWidget* itShow = gtk_check_menu_item_new_with_label(_("show"));
		gtk_menu_attach(GTK_MENU(menu), itShow, 2, 3, menuY, menuY + 1);
		gtk_widget_set_hexpand(itShow, false);

		showLayerItems[layer] = itShow;

		menuY++;
	}

	if (layer > 0)
	{
		gtk_menu_attach(GTK_MENU(menu), gtk_separator_menu_item_new(), 0, MENU_WIDTH, menuY, menuY + 1);
		menuY++;
	}

	createLayerMenuItem(_("Background"), 0);

	GtkWidget* itShowBackground = gtk_check_menu_item_new_with_label(_("show"));
	gtk_menu_attach(GTK_MENU(menu), itShowBackground, 2, 3, menuY, menuY + 1);
	gtk_widget_set_hexpand(itShowBackground, false);
	showLayerItems[0] = itShowBackground;
	menuY++;

	gtk_widget_show_all(menu);

	updateLayerData();
}

/**
 * Update selected layer, update visible layer
 */
void ToolPageLayer::updateLayerData()
{
	XOJ_CHECK_TYPE(ToolPageLayer);

	int layerId = lc->getCurrentLayerId();

	inMenuUpdate = true;

	for (auto& kv : layerItems)
	{
		if (kv.first == layerId)
		{
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(kv.second), true);
		}
		else
		{
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(kv.second), false);
		}
	}

	inMenuUpdate = false;

	string lb;
	if (layerId > 0)
	{
		lb = FS(_F("Layer {1}") % layerId);
	}
	else
	{
		lb = _("Background");
	}
	gtk_label_set_text(GTK_LABEL(layerLabel), lb.c_str());
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
