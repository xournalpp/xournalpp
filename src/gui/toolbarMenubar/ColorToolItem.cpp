#include "ColorToolItem.h"
#include "../widgets/SelectColor.h"
#include <Util.h>

#include <config.h>
#include <glib/gi18n-lib.h>

bool ColorToolItem::inUpdate = false;

ColorToolItem::ColorToolItem(String id, ActionHandler * handler, ToolHandler * toolHandler, int color, String name, bool selector) :
	AbstractToolItem(id, handler, selector ? ACTION_SELECT_COLOR_CUSTOM : ACTION_SELECT_COLOR) {
	XOJ_INIT_TYPE(ColorToolItem);

	this->color = color;
	this->selector = selector;
	this->toolHandler = toolHandler;
	this->group = GROUP_COLOR;
	this->colorDlg = NULL;
	this->name = name;
}

ColorToolItem::~ColorToolItem() {
	XOJ_RELEASE_TYPE(ColorToolItem);

	if (this->iconWidget) {
		g_object_ref(this->iconWidget);
	}
}

void ColorToolItem::actionSelected(ActionGroup group, ActionType action) {
	XOJ_CHECK_TYPE(ColorToolItem);

	inUpdate = true;
	if (this->group == group && item) {
		if (selector) {
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), action == ACTION_SELECT_COLOR_CUSTOM);
		}
		enableColor(toolHandler->getColor());
	}
	inUpdate = false;
}

void ColorToolItem::enableColor(int color) {
	XOJ_CHECK_TYPE(ColorToolItem);

	if (selector) {
		selectcolor_set_color(iconWidget, color);
		this->color = color;
		if (GTK_IS_TOGGLE_BUTTON(item)) {
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), false);
		}
	} else {
		bool active = colorEqualsMoreOreLess(color);

		if (item) {
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), active);
		}

		if (active) {
			this->toolHandler->setColorFound();

			// Only equals more ore less, so we will set it exact to the default color
			if (this->color != color) {
				this->toolHandler->setColor(this->color);
			}

		}
	}
}

void ColorToolItem::selectColor() {
	XOJ_CHECK_TYPE(ColorToolItem);

	GdkColor color = { 0 };

	GtkWidget * cw = gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(this->colorDlg));
	gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(cw), &color);

	this->color = Util::gdkColorToInt(color);
}

bool ColorToolItem::colorEqualsMoreOreLess(int color) {
	XOJ_CHECK_TYPE(ColorToolItem);

	if (color == -1) {
		return false;
	}

	int r1 = (color & 0xff0000) >> 16;
	int g1 = (color & 0xff00) >> 8;
	int b1 = (color & 0xff);

	int r2 = (this->color & 0xff0000) >> 16;
	int g2 = (this->color & 0xff00) >> 8;
	int b2 = (this->color & 0xff);

	if (abs(r1 - r2) < 10 && abs(g1 - g2) < 10 && abs(b1 - b2) < 10) {
		return true;
	}
	return false;
}

void ColorToolItem::activated(GdkEvent *event, GtkMenuItem *menuitem, GtkToolButton *toolbutton) {
	XOJ_CHECK_TYPE(ColorToolItem);

	if (inUpdate) {
		return;
	}
	inUpdate = true;

	if (selector) {
		this->colorDlg = gtk_color_selection_dialog_new(_("Select color"));
		g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(this->colorDlg)->ok_button), "clicked", G_CALLBACK(&customColorSelected), this);

		GdkColor color = Util::intToGdkColor(this->color);

		GtkWidget* cw = gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(this->colorDlg));
		gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(cw), &color);

		gtk_dialog_run(GTK_DIALOG(this->colorDlg));

		gtk_widget_destroy(this->colorDlg);
		this->colorDlg = NULL;
	}

	toolHandler->setColor(color);

	inUpdate = false;
}


// TODO LOW PRIO: add history
//static void
//change_color_callback (GtkWidget *button,
//		       gpointer	  data)
//{
//  GtkWidget *dialog;
//  GtkColorSelection *colorsel;
//  gint response;
//
//  dialog = gtk_color_selection_dialog_new ("Changing color");
//
//  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
//
//  colorsel = GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel);
//
//  gtk_color_selection_set_previous_color (colorsel, &color);
//  gtk_color_selection_set_current_color (colorsel, &color);
//  gtk_color_selection_set_has_palette (colorsel, TRUE);
//
//  response = gtk_dialog_run (GTK_DIALOG (dialog));
//
//  if (response == GTK_RESPONSE_OK)
//    {
//      gtk_color_selection_get_current_color (colorsel,
//					     &color);
//
//      gtk_widget_modify_bg (da, GTK_STATE_NORMAL, &color);
//    }
//
//  gtk_widget_destroy (dialog);
//}

GtkToolItem * ColorToolItem::newItem() {
	XOJ_CHECK_TYPE(ColorToolItem);

	this->iconWidget = selectcolor_new(color);

	selectcolor_set_circle(this->iconWidget, !this->selector);
	GtkToolItem * it = gtk_toggle_tool_button_new();

	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(it), this->name.c_str());
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), this->name.c_str());

	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it), this->iconWidget);

	return it;
}

void ColorToolItem::customColorSelected(GtkWidget * button, ColorToolItem * item) {
	XOJ_CHECK_TYPE_OBJ(item, ColorToolItem);

	item->selectColor();
}

String ColorToolItem::getToolDisplayName() {
	XOJ_CHECK_TYPE(ColorToolItem);

	return this->name;
}

GtkWidget * ColorToolItem::getNewToolIcon() {
	XOJ_CHECK_TYPE(ColorToolItem);

	GtkWidget * iconWidget = selectcolor_new(color);
	selectcolor_set_circle(iconWidget, !this->selector);

	return iconWidget;
}

