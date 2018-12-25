#include "SidebarPreviewLayerEntry.h"

#include "gui/Shadow.h"
#include "gui/sidebar/previews/layer/SidebarPreviewLayers.h"

#include <i18n.h>


SidebarPreviewLayerEntry::SidebarPreviewLayerEntry(SidebarPreviewBase* sidebar, PageRef page, int layer, size_t index)
 : SidebarPreviewBaseEntry(sidebar, page),
   box(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2)),
   toolbarHeight(0),
   index(index)
{
	XOJ_INIT_TYPE(SidebarPreviewLayerEntry);
	this->layer = layer;

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

	GtkWidget* show = gtk_check_button_new_with_label(text.c_str());

	// Left padding
	gtk_widget_set_margin_start(show, Shadow::getShadowTopLeftSize());

	gtk_container_add(GTK_CONTAINER(toolbar), show);

	gtk_widget_set_vexpand(widget, false);
	gtk_container_add(GTK_CONTAINER(box), widget);

	gtk_widget_set_vexpand(toolbar, false);
	gtk_container_add(GTK_CONTAINER(box), toolbar);

	gtk_widget_show_all(box);

	toolbarHeight = gtk_widget_get_allocated_height(show) + Shadow::getShadowTopLeftSize() + 20;
}

SidebarPreviewLayerEntry::~SidebarPreviewLayerEntry()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);

	gtk_widget_destroy(this->box);
	this->box = NULL;

	XOJ_RELEASE_TYPE(SidebarPreviewLayerEntry);
}

void SidebarPreviewLayerEntry::mouseButtonPressCallback()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);

	((SidebarPreviewLayers*)sidebar)->layerSelected(index);
}

PreviewRenderType SidebarPreviewLayerEntry::getRenderType()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);

	return RENDER_TYPE_PAGE_LAYER;
}

int SidebarPreviewLayerEntry::getHeight()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);

	return getWidgetHeight() + toolbarHeight;
}

int SidebarPreviewLayerEntry::getLayer()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);

	return layer;
}

GtkWidget* SidebarPreviewLayerEntry::getWidget()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);

	return this->box;
}
