#include "SidebarPreviewLayerEntry.h"

#include "gui/Shadow.h"
#include "gui/sidebar/previews/layer/SidebarPreviewLayers.h"

#include <i18n.h>


SidebarPreviewLayerEntry::SidebarPreviewLayerEntry(SidebarPreviewBase* sidebar, PageRef page, int layer, size_t index)
 : SidebarPreviewBaseEntry(sidebar, page),
   index(index),
   layer(layer),
   box(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2))
{
	GtkWidget* toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);

	string text;
	if (layer < 0)
	{
		text = _("Background");
	}
	else
	{
		text = FS(_F("Layer {1}") % (layer + 1));
	}

	cbVisible = gtk_check_button_new_with_label(text.c_str());

	g_signal_connect(cbVisible, "toggled", G_CALLBACK(
		+[](GtkToggleButton* source, SidebarPreviewLayerEntry* self)
		{
	self->checkboxToggled();
		}), this);


	// Left padding
	gtk_widget_set_margin_start(cbVisible, Shadow::getShadowTopLeftSize());

	gtk_container_add(GTK_CONTAINER(toolbar), cbVisible);

	gtk_widget_set_vexpand(widget, false);
	gtk_container_add(GTK_CONTAINER(box), widget);

	gtk_widget_set_vexpand(toolbar, false);
	gtk_container_add(GTK_CONTAINER(box), toolbar);

	gtk_widget_show_all(box);

	toolbarHeight = gtk_widget_get_allocated_height(cbVisible) + Shadow::getShadowTopLeftSize() + 20;
}

SidebarPreviewLayerEntry::~SidebarPreviewLayerEntry()
{
	gtk_widget_destroy(this->box);
	this->box = nullptr;
}

void SidebarPreviewLayerEntry::checkboxToggled()
{
	if (inUpdate)
	{
		return;
	}

	bool check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbVisible));
	((SidebarPreviewLayers*)sidebar)->layerVisibilityChanged(layer + 1, check);
}

void SidebarPreviewLayerEntry::mouseButtonPressCallback()
{
	((SidebarPreviewLayers*)sidebar)->layerSelected(index);
}

PreviewRenderType SidebarPreviewLayerEntry::getRenderType()
{
	return RENDER_TYPE_PAGE_LAYER;
}

int SidebarPreviewLayerEntry::getHeight()
{
	return getWidgetHeight() + toolbarHeight;
}

int SidebarPreviewLayerEntry::getLayer()
{
	return layer;
}

GtkWidget* SidebarPreviewLayerEntry::getWidget()
{
	return this->box;
}

/**
 * Set the value of the visible checkbox
 */
void SidebarPreviewLayerEntry::setVisibleCheckbox(bool enabled)
{
	inUpdate = true;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbVisible), enabled);

	inUpdate = false;
}

