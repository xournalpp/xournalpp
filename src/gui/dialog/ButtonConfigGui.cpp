#include "ButtonConfigGui.h"

#include "SettingsDialog.h"
#include "control/settings/Settings.h"
#include "control/settings/ButtonConfig.h"

#include <config.h>
#include <i18n.h>

#define	ADD_TYPE_CB(icon, name, action) \
	gtk_list_store_append(typeModel, &iter); \
	gtk_list_store_set(typeModel, &iter, 0, dlg->loadIconPixbuf(icon), 1, name, 2, action, -1);

ButtonConfigGui::ButtonConfigGui(SettingsDialog* dlg, GtkWidget* w,  Settings* settings, int button, bool withDevice)
{
	XOJ_INIT_TYPE(ButtonConfigGui);

	this->settings = settings;
	this->button = button;
	this->withDevice = withDevice;

	GtkWidget* table = gtk_table_new(8, 2, false);

	if (withDevice)
	{
		this->cbDevice = gtk_combo_box_new_text();

		gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbDevice), _C("No device"));

		GList* devices = gdk_devices_list();
		for (GList* l = devices; l != NULL; l = l->next)
		{
			GdkDevice* dev = (GdkDevice*) l->data;

			string devType = "";
			if (dev->source == GDK_SOURCE_MOUSE)
			{
				devType = _("mouse");
			}
			else if (dev->source == GDK_SOURCE_PEN)
			{
				devType = _("pen");
			}
			else if (dev->source == GDK_SOURCE_ERASER)
			{
				devType = _("eraser");
			}
			else if (dev->source == GDK_SOURCE_CURSOR)
			{
				devType = _("cursor");
			}

			char* txt = g_strdup_printf("%s (%s)", dev->name, devType.c_str());
			gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbDevice), txt);
			g_free(txt);
		}

		gtk_table_attach(GTK_TABLE(table), newLabel(_C("Device")), 0, 1, 0, 1, GTK_FILL, GTK_FILL, 20, 0);
		gtk_table_attach(GTK_TABLE(table), this->cbDevice, 1, 2, 0, 1,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

		cbDisableDrawing = gtk_check_button_new_with_label(_C("Disable drawing for this device"));

		gtk_table_attach(GTK_TABLE(table), cbDisableDrawing, 1, 2, 1, 2,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	}
	else
	{
		this->cbDevice = NULL;
		this->cbDisableDrawing = NULL;
	}

	GtkListStore* typeModel = gtk_list_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_INT);
	GtkTreeIter iter;

	ADD_TYPE_CB("empty.svg",            _C("Don't change"),     TOOL_NONE);
	ADD_TYPE_CB("tool_pencil.svg",      _C("Pen"),              TOOL_PEN);
	ADD_TYPE_CB("tool_eraser.svg",      _C("Eraser"),           TOOL_ERASER);
	ADD_TYPE_CB("tool_highlighter.svg", _C("Highlighter"),      TOOL_HILIGHTER);
	ADD_TYPE_CB("tool_text.svg",        _C("Text"),             TOOL_TEXT);
	ADD_TYPE_CB("tool_image.svg",       _C("Insert image"),     TOOL_IMAGE);
	ADD_TYPE_CB("stretch.svg",          _C("Vertical space"),   TOOL_VERTICAL_SPACE);
	ADD_TYPE_CB("lasso.svg",            _C("Select region"),    TOOL_SELECT_REGION);
	ADD_TYPE_CB("rect-select.svg",      _C("Select rectangle"), TOOL_SELECT_RECT);
	//ADD_TYPE_CB("rect-draw.svg",      _C("Draw rectangle"),   TOOL_DRAW_RECT);
	//ADD_TYPE_CB("circle-draw.svg",    _C("Draw circle"),      TOOL_DRAW_CIRCLE);
	ADD_TYPE_CB("hand.svg",             _C("Hand"),             TOOL_HAND);

	GtkCellRenderer* renderer = NULL;

	this->cbTool = gtk_combo_box_new_with_model(GTK_TREE_MODEL(typeModel));
	g_signal_connect(cbTool, "changed", G_CALLBACK(&cbSelectCallback), this);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(this->cbTool), renderer, false);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(this->cbTool), renderer, "pixbuf", 0, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(this->cbTool), renderer, true);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(this->cbTool), renderer, "text", 1, NULL);

	gtk_table_attach(GTK_TABLE(table), newLabel(_C("Tool")), 0, 1, 2, 3, GTK_FILL, GTK_FILL, 20, 0);
	gtk_table_attach(GTK_TABLE(table), cbTool, 1, 2, 2, 3,
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	this->cbThickness = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbThickness), _C("Don't change"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbThickness), _C("Thin"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbThickness), _C("Medium"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(cbThickness), _C("Thick"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbThickness), 0);

	gtk_table_attach(GTK_TABLE(table), newLabel(_C("Thickness")), 0, 1, 3, 4,
					GTK_FILL, GTK_FILL, 20, 0);
	gtk_table_attach(GTK_TABLE(table), this->cbThickness, 1, 2, 3, 4,
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	this->colorButton = gtk_color_button_new();
	gtk_table_attach(GTK_TABLE(table), newLabel(_C("Color")), 0, 1, 4, 5, GTK_FILL, GTK_FILL, 20, 0);
	gtk_table_attach(GTK_TABLE(table), colorButton, 1, 2, 4, 5,
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	this->cbDrawingType = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbDrawingType), _C("Don't change"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbDrawingType), _C("Enable Ruler"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbDrawingType), _C("Enable Stroke Recognizer"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbDrawingType), _C("Disable Ruler & Stroke Recognizer"));

	gtk_table_attach(GTK_TABLE(table), newLabel(_C("Ruler & Stroke Reco.")), 0, 1, 5, 6, GTK_FILL, GTK_FILL, 20, 0);
	gtk_table_attach(GTK_TABLE(table), this->cbDrawingType, 1, 2, 5, 6,
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	this->cbEraserType = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbEraserType), _C("Don't change"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbEraserType), _C("Standard"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbEraserType), _C("Whiteout"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(this->cbEraserType), _C("Delete stroke"));

	gtk_table_attach(GTK_TABLE(table), newLabel(_C("Eraser type")), 0, 1, 8, 9, GTK_FILL, GTK_FILL, 20, 0);
	gtk_table_attach(GTK_TABLE(table), this->cbEraserType, 1, 2, 8, 9,
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	gtk_container_add(GTK_CONTAINER(w), table);
	gtk_widget_show_all(table);

	loadSettings();
}

ButtonConfigGui::~ButtonConfigGui()
{
	XOJ_RELEASE_TYPE(ButtonConfigGui);
}

void ButtonConfigGui::loadSettings()
{
	XOJ_CHECK_TYPE(ButtonConfigGui);

	ButtonConfig* cfg = settings->getButtonConfig(button);

	GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(cbTool));
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
	{
		return;
	}

	GValue value = {0};
	int i = 0;

	gtk_combo_box_set_active(GTK_COMBO_BOX(cbTool), 0);

	do
	{
		gtk_tree_model_get_value(model, &iter, 2, &value);

		int action = g_value_get_int(&value);

		if (action == cfg->action)
		{
			gtk_combo_box_set_active(GTK_COMBO_BOX(cbTool), i);
			break;
		}

		i++;
		g_value_unset(&value);
	}
	while (gtk_tree_model_iter_next(model, &iter));

	if (cfg->size == TOOL_SIZE_FINE)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbThickness), 1);
	}
	else if (cfg->size == TOOL_SIZE_MEDIUM)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbThickness), 2);
	}
	else if (cfg->size == TOOL_SIZE_THICK)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbThickness), 3);
	}
	else
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbThickness), 0);
	}

	GdkColor color = {0, 0, 0, 0};
	color.red = (cfg->color >> 8) & 0xff00;
	color.green = (cfg->color >> 0) & 0xff00;
	color.blue = (cfg->color << 8) & 0xff00;

	gtk_color_button_set_color(GTK_COLOR_BUTTON(colorButton), &color);

	gtk_combo_box_set_active(GTK_COMBO_BOX(this->cbDrawingType), cfg->drawingType);

	if (cfg->eraserMode == ERASER_TYPE_DEFAULT)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbEraserType), 1);
	}
	else if (cfg->eraserMode == ERASER_TYPE_WHITEOUT)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbEraserType), 2);
	}
	else if (cfg->eraserMode == ERASER_TYPE_DELETE_STROKE)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbEraserType), 3);
	}
	else
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbEraserType), 0);
	}

	if (withDevice)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(cbDevice), 0);

		int i = 0;
		GList* devices = gdk_devices_list();
		for (GList* l = devices; l != NULL; l = l->next, i++)
		{
			GdkDevice* dev = (GdkDevice*) l->data;
			if (cfg->device == dev->name)
			{
				gtk_combo_box_set_active(GTK_COMBO_BOX(cbDevice), i + 1);
				break;
			}
		}

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbDisableDrawing), cfg->disableDrawing);
	}
}

void ButtonConfigGui::saveSettings()
{
	XOJ_CHECK_TYPE(ButtonConfigGui);

	ButtonConfig* cfg = settings->getButtonConfig(button);
	ToolType action = TOOL_NONE;
	GtkTreeIter iter;

	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(cbTool), &iter);

	GValue value = {0};
	GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(cbTool));

	gtk_tree_model_get_value(model, &iter, 2, &value);
	action = (ToolType) g_value_get_int(&value);

	cfg->action = action;

	int thickness = gtk_combo_box_get_active(GTK_COMBO_BOX(cbThickness));

	if (thickness == 1)
	{
		cfg->size = TOOL_SIZE_FINE;
	}
	else if (thickness == 2)
	{
		cfg->size = TOOL_SIZE_MEDIUM;
	}
	else if (thickness == 3)
	{
		cfg->size = TOOL_SIZE_THICK;
	}
	else
	{
		cfg->size = TOOL_SIZE_NONE;
	}

	GdkColor color = {0, 0, 0, 0};
	gtk_color_button_get_color(GTK_COLOR_BUTTON(colorButton), &color);

	cfg->color = (color.red / 256) << 16 | (color.green / 256) << 8 | (color.blue / 256);

	cfg->drawingType = (DrawingType) gtk_combo_box_get_active(GTK_COMBO_BOX(this->cbDrawingType));

	int eraserMode = gtk_combo_box_get_active(GTK_COMBO_BOX(this->cbEraserType));

	if (eraserMode == 1)
	{
		cfg->eraserMode = ERASER_TYPE_DEFAULT;
	}
	else if (eraserMode == 2)
	{
		cfg->eraserMode = ERASER_TYPE_WHITEOUT;
	}
	else if (eraserMode == 3)
	{
		cfg->eraserMode = ERASER_TYPE_DELETE_STROKE;
	}
	else
	{
		cfg->eraserMode = ERASER_TYPE_NONE;
	}

	if (this->withDevice)
	{
		GList* devices = gdk_devices_list();
		int dev = gtk_combo_box_get_active(GTK_COMBO_BOX(cbDevice));
		GList* selected = g_list_nth(devices, dev - 1);
		if (selected == 0)
		{
			cfg->device = "";
		}
		else
		{
			cfg->device = ((GdkDevice*) selected->data)->name;
		}

		cfg->disableDrawing = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbDisableDrawing));
	}

	settings->customSettingsChanged();
}

GtkWidget* ButtonConfigGui::newLabel(const char* text)
{
	GtkWidget* label = gtk_label_new(text);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	return label;
}

void ButtonConfigGui::cbSelectCallback(GtkComboBox* widget, ButtonConfigGui* gui)
{
	XOJ_CHECK_TYPE_OBJ(gui, ButtonConfigGui);
	gui->enableDisableTools();
}

void ButtonConfigGui::enableDisableTools()
{
	XOJ_CHECK_TYPE(ButtonConfigGui);

	ToolType action = TOOL_NONE;
	GtkTreeIter iter;

	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(cbTool), &iter);

	GValue value = {0};
	GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(cbTool));

	gtk_tree_model_get_value(model, &iter, 2, &value);
	action = (ToolType) g_value_get_int(&value);

	switch (action)
	{
	case TOOL_PEN:
	case TOOL_HILIGHTER:
		gtk_widget_set_sensitive(cbThickness, true);
		gtk_widget_set_sensitive(colorButton, true);
		gtk_widget_set_sensitive(cbDrawingType, true);
		gtk_widget_set_sensitive(cbEraserType, false);
		break;
		
	case TOOL_ERASER:
		gtk_widget_set_sensitive(cbThickness, true);
		gtk_widget_set_sensitive(colorButton, false);
		gtk_widget_set_sensitive(cbDrawingType, false);
		gtk_widget_set_sensitive(cbEraserType, true);
		break;
		
	case TOOL_TEXT:
		gtk_widget_set_sensitive(cbThickness, false);
		gtk_widget_set_sensitive(colorButton, true);
		gtk_widget_set_sensitive(cbDrawingType, false);
		gtk_widget_set_sensitive(cbEraserType, false);
		break;
		
	case TOOL_NONE:
	case TOOL_IMAGE:
		//case TOOL_DRAW_RECT:
		//case TOOL_DRAW_CIRCLE:
	case TOOL_SELECT_RECT:
	case TOOL_SELECT_REGION:
	case TOOL_VERTICAL_SPACE:
	case TOOL_HAND:
		gtk_widget_set_sensitive(cbThickness, false);
		gtk_widget_set_sensitive(colorButton, false);
		gtk_widget_set_sensitive(cbDrawingType, false);
		gtk_widget_set_sensitive(cbEraserType, false);
		break;
	}
}
