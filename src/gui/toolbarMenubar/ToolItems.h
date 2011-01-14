/*
 * Xournal Extended
 *
 * Part of the customizeable toolbars
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#ifndef __TOOLITEMS_H__
#define __TOOLITEMS_H__

#include <gtk/gtk.h>
#include "../widgets/SelectColor.h"
#include "../../control/ToolHandler.h"
#include "../../control/Actions.h"
#include "../../control/ZoomControl.h"
#include "../../model/String.h"
#include "../GladeGui.h"
#include "../widgets/gtkmenutooltogglebutton.h"
#include "../../util/Util.h"

#include <stdlib.h>

class AbstractItem;

/**
 * An abstact action item, may with a menuitem
 */
class AbstractItem: public ActionEnabledListener, public ActionSelectionListener {
public:
	AbstractItem(String id, ActionHandler * handler, ActionType action, GtkWidget * menuitem = NULL) {
		this->id = id;
		this->handler = handler;
		this->action = action;
		this->menuitem = NULL;
		this->menuSignalHandler = 0;
		this->group = GROUP_NOGROUP;
		this->enabled = true;

		ActionEnabledListener::registerListener(handler);
		ActionSelectionListener::registerListener(handler);

		if (menuitem) {
			menuSignalHandler = g_signal_connect(menuitem, "activate", G_CALLBACK(&menuCallback), this);
			gtk_object_ref(GTK_OBJECT(menuitem));
			this->menuitem = menuitem;
		}
	}

	virtual ~AbstractItem() {
		if (this->menuitem) {
			g_signal_handler_disconnect(this->menuitem, menuSignalHandler);
			gtk_object_unref(GTK_OBJECT(this->menuitem));
		}
	}

	static void menuCallback(GtkMenuItem *menuitem, AbstractItem * toolItem) {
		toolItem->activated(NULL, menuitem, NULL);
	}

	virtual void actionSelected(ActionGroup group, ActionType action) {
		if (this->group == group) {

			if (this->menuitem && GTK_IS_CHECK_MENU_ITEM(this->menuitem)) {
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(this->menuitem)) != (this->action == action)) {
					gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(this->menuitem), this->action == action);
				}
			}
			selected(group, action);
		}
	}

	/**
	 * Override this method
	 */
	virtual void selected(ActionGroup group, ActionType action) {
	}

	virtual void actionEnabledAction(ActionType action, bool enabled) {
		if (this->action == action) {
			this->enabled = enabled;
			enable(enabled);
			if (this->menuitem) {
				gtk_widget_set_sensitive(GTK_WIDGET(this->menuitem), enabled);
			}
		}
	}

	virtual void activated(GdkEvent *event, GtkMenuItem *menuitem, GtkToolButton *toolbutton) {
		bool selected = true;

		if (menuitem && GTK_IS_CHECK_MENU_ITEM(menuitem)) {
			selected = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
		} else if (toolbutton && GTK_IS_TOGGLE_TOOL_BUTTON(toolbutton)) {
			selected = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton));
		}

		handler->actionPerformed(action, group, event, menuitem, toolbutton, selected);
	}

	String getId() {
		return id;
	}

	void setTmpDisabled(bool disabled) {
		bool ena = false;
		if (disabled) {
			ena = false;
		} else {
			ena = this->enabled;
		}

		enable(ena);
		if (this->menuitem) {
			gtk_widget_set_sensitive(GTK_WIDGET(this->menuitem), ena);
		}
	}

protected:
	virtual void enable(bool enabled) {
	}

	ActionGroup group;
	ActionType action;

	String id;

	ActionHandler * handler;

	bool enabled;
private:
	gulong menuSignalHandler;
	GtkWidget * menuitem;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/**
 * Menuitem handler
 */
class MenuItem: public AbstractItem {
public:
	MenuItem(ActionHandler * handler, GtkWidget * widget, ActionType type) :
		AbstractItem(NULL, handler, type, widget) {
	}

	MenuItem(ActionHandler * handler, GtkWidget * widget, ActionType type, ActionGroup group) :
		AbstractItem(NULL, handler, type, widget) {
		this->group = group;
	}

	virtual ~MenuItem() {
	}
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/**
 * Tool item handler
 */
class AbstractToolItem: public AbstractItem {
public:
	AbstractToolItem(String id, ActionHandler * handler, ActionType type, GtkWidget * menuitem = NULL) :
		AbstractItem(id, handler, type, menuitem) {
		this->item = NULL;
		this->popupMenu = NULL;
		this->used = false;
	}

	virtual ~AbstractToolItem() {
		if (item) {
			gtk_object_unref(GTK_OBJECT(item));
		}
		if (this->popupMenu) {
			gtk_object_unref(GTK_OBJECT(this->popupMenu));
		}
	}

	virtual void selected(ActionGroup group, ActionType action) {
		if (item) {
			if (!GTK_IS_TOGGLE_TOOL_BUTTON(item)) {
				g_warning("selected action %i which is not a toggle action!", action);
				return;
			}
			if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(item)) != (this->action == action)) {
				gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), (this->action == action));
			}
		}
	}

	static void toolButtonCallback(GtkToolButton *toolbutton, AbstractToolItem * item) {
		item->activated(NULL, NULL, toolbutton);
	}

	virtual GtkToolItem * createItem(bool horizontal) {
		if (item) {
			return item;
		}

		item = newItem();
		g_object_ref(item);

		if (GTK_IS_TOOL_ITEM(item)) {
			gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
		}
		if (GTK_IS_TOOL_BUTTON(item) || GTK_IS_TOGGLE_TOOL_BUTTON(item)) {
			g_signal_connect(item, "clicked", G_CALLBACK(&toolButtonCallback), this);
		}
		gtk_object_ref(GTK_OBJECT(item));
		return item;

	}

	void setPopupMenu(GtkWidget * popupMenu) {
		if (this->popupMenu) {
			g_object_unref(this->popupMenu);
		}
		if (popupMenu) {
			g_object_ref(popupMenu);
		}

		this->popupMenu = popupMenu;
	}

	GtkWidget * getPopupMenu() {
		return this->popupMenu;
	}

	bool isUsed() {
		return used;
	}

	void setUsed(bool used) {
		this->used = used;
	}

protected:
	virtual GtkToolItem * newItem() = 0;

	virtual void enable(bool enabled) {
		if (this->item) {
			gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
		}
	}

protected:
	GtkToolItem * item;
	GtkWidget * popupMenu;

	bool used;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class ColorToolItem;

static void customColorSelected(GtkWidget *button, ColorToolItem * item);

class ColorToolItem: public AbstractToolItem {
public:
	ColorToolItem(String id, ActionHandler * handler, ToolHandler * toolHandler, gint color, bool selector = false) :
		AbstractToolItem(id, handler, selector ? ACTION_SELECT_COLOR_CUSTOM : ACTION_SELECT_COLOR) {
		this->color = color;
		this->selector = selector;
		this->toolHandler = toolHandler;
		this->group = GROUP_COLOR;
		this->colorDlg = NULL;
	}

	virtual void actionSelected(ActionGroup group, ActionType action) {
		inUpdate = true;
		if (this->group == group && item) {
			if (selector) {
				gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), action == ACTION_SELECT_COLOR_CUSTOM);
			}
			enableColor(toolHandler->getColor());
		}
		inUpdate = false;
	}

	void enableColor(int color) {
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

	void selectColor() {
		GdkColor color = { 0 };

		GtkWidget* cw = gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(colorDlg));
		gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(cw), &color);

		this->color = Util::gdkColorToInt(color);
	}

	bool colorEqualsMoreOreLess(int color) {
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

	virtual ~ColorToolItem() {
	}

	virtual void activated(GdkEvent *event, GtkMenuItem *menuitem, GtkToolButton *toolbutton) {
		if (inUpdate) {
			return;
		}
		inUpdate = true;

		if (selector) {
			colorDlg = gtk_color_selection_dialog_new(_("Select color"));
			g_signal_connect(G_OBJECT
					(GTK_COLOR_SELECTION_DIALOG(colorDlg)->ok_button),
					"clicked", G_CALLBACK(&customColorSelected), this);

			GdkColor color = Util::intToGdkColor(this->color);

			GtkWidget* cw = gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(colorDlg));
			gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(cw), &color);

			gtk_dialog_run(GTK_DIALOG(colorDlg));

			gtk_widget_destroy(colorDlg);
			colorDlg = NULL;
		}

		toolHandler->setColor(color);

		inUpdate = false;
	}

protected:
	virtual GtkToolItem * newItem() {
		iconWidget = selectcolor_new(color);
		selectcolor_set_circle(iconWidget, !selector);
		GtkToolItem * it = gtk_toggle_tool_button_new();

		if (selector) {
			gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), _("Select color"));
		} else {
			gchar * name = g_strdup_printf("%06x", color);
			gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), name);
			g_free(name);
		}
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it), iconWidget);

		return it;
	}

private:
	gint color;
	bool selector;
	GtkWidget * iconWidget;
	GtkWidget * colorDlg;

	ToolHandler * toolHandler;

	static bool inUpdate;
};

bool ColorToolItem::inUpdate = false;

static void customColorSelected(GtkWidget *button, ColorToolItem * item) {
	item->selectColor();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


class ToolButton: public AbstractToolItem {
public:
	ToolButton(ActionHandler * handler, String id, ActionType type, String stock, String description,
			GtkWidget * menuitem = NULL) :
		AbstractToolItem(id, handler, type, menuitem) {
		this->stock = stock;
		this->gui = NULL;
		this->description = description;
	}

	ToolButton(ActionHandler * handler, GladeGui *gui, String id, ActionType type, String iconName, String description,
			GtkWidget * menuitem = NULL) :
		AbstractToolItem(id, handler, type, menuitem) {
		this->iconName = iconName;
		this->gui = gui;
		this->description = description;
	}

	ToolButton(ActionHandler * handler, GladeGui *gui, String id, ActionType type, ActionGroup group, String iconName,
			String description, GtkWidget * menuitem = NULL) :
		AbstractToolItem(id, handler, type, menuitem) {
		this->iconName = iconName;
		this->gui = gui;
		this->description = description;
		this->group = group;
	}

	virtual ~ToolButton() {
	}

	void updateDescription(String description) {
		this->description = description;
		if (GTK_IS_TOOL_ITEM(item)) {
			gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), description.c_str());
		}
	}

protected:
	virtual GtkToolItem * newItem() {
		GtkToolItem * it;

		if (!stock.isEmpty()) {
			if (popupMenu) {
				it = gtk_menu_tool_button_new_from_stock(stock.c_str());
				gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(it), popupMenu);
			} else {
				it = gtk_tool_button_new_from_stock(stock.c_str());
			}
		} else if (group != GROUP_NOGROUP) {
			if (popupMenu) {
				it = gtk_menu_tool_toggle_button_new(gui->loadIcon(iconName.c_str()), description.c_str());
				gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popupMenu);
			} else {
				it = gtk_toggle_tool_button_new();
				gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it), gui->loadIcon(iconName.c_str()));
			}
		} else {
			if (popupMenu) {
				it = gtk_menu_tool_button_new(gui->loadIcon(iconName.c_str()), description.c_str());
				gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(it), popupMenu);
			} else {
				it = gtk_tool_button_new(gui->loadIcon(iconName.c_str()), description.c_str());
			}
		}
		gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (it), description.c_str());
		return it;
	}

protected:
	GladeGui * gui;

private:
	String stock;

	String iconName;
	String description;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class ToolMenuHandler;

class ToolSelectCombocontrol: public ToolButton {
public:
	ToolSelectCombocontrol(ToolMenuHandler * th, ActionHandler * handler, GladeGui *gui, String id);

	virtual ~ToolSelectCombocontrol() {
		g_object_unref(this->iconSelectRect);
		g_object_unref(this->iconSelectRgion);
		g_object_unref(this->iconSelectObject);
	}

	virtual void selected(ActionGroup group, ActionType action) {
		if (item) {
			if (!GTK_IS_TOGGLE_TOOL_BUTTON(item)) {
				g_warning("selected action %i which is not a toggle action! 2", action);
				return;
			}

			if (action == ACTION_TOOL_SELECT_RECT && this->action != ACTION_TOOL_SELECT_RECT) {
				this->action = ACTION_TOOL_SELECT_RECT;
				gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconSelectRect);

				gtk_tool_button_set_label(GTK_TOOL_BUTTON(item), _("Select Rectangle"));
			} else if (action == ACTION_TOOL_SELECT_REGION && this->action != ACTION_TOOL_SELECT_REGION) {
				this->action = ACTION_TOOL_SELECT_REGION;
				gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconSelectRgion);
				gtk_tool_button_set_label(GTK_TOOL_BUTTON(item), _("Select Region"));
			} else if (action == ACTION_TOOL_SELECT_OBJECT && this->action != ACTION_TOOL_SELECT_OBJECT) {
				this->action = ACTION_TOOL_SELECT_OBJECT;
				gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconSelectObject);
				gtk_tool_button_set_label(GTK_TOOL_BUTTON(item), _("Select Object"));
			}

			if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(item)) != (this->action == action)) {
				gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), (this->action == action));
			}
		}
	}

protected:
	virtual GtkToolItem * newItem() {
		GtkToolItem * it;

		labelWidget = gtk_label_new(_("Select Rectangle"));
		iconWidget = gtk_image_new_from_pixbuf(this->iconSelectRect);

		it = gtk_menu_tool_toggle_button_new(iconWidget, "test0");
		gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
		gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popupMenu);
		return it;
	}

private:
	GtkWidget * iconWidget;
	GtkWidget * labelWidget;

	GdkPixbuf * iconSelectRect;
	GdkPixbuf * iconSelectRgion;
	GdkPixbuf * iconSelectObject;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


class ToolPageSpinner: public AbstractToolItem {
public:
	ToolPageSpinner(ActionHandler * handler, String id, ActionType type) :
		AbstractToolItem(id, handler, type, NULL) {
		pageSpinner = gtk_spin_button_new_with_range(0, 0, 1);
		lbPageNo = NULL;
	}

	virtual ~ToolPageSpinner() {
	}

	GtkWidget * getPageSpinner() {
		return pageSpinner;
	}

	void setText(String text) {
		if (lbPageNo) {
			gtk_label_set_text(GTK_LABEL(lbPageNo), text.c_str());
		}
	}

protected:
	virtual GtkToolItem * newItem() {
		GtkToolItem * it = gtk_tool_item_new();

		GtkWidget * hbox = gtk_hbox_new(false, 1);
		gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_("Page")), false, false, 7);

		gtk_box_pack_start(GTK_BOX(hbox), pageSpinner, false, false, 0);

		lbPageNo = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(hbox), lbPageNo, false, false, 0);

		gtk_container_add(GTK_CONTAINER (it), hbox);

		return it;
	}

private:
	GtkWidget * pageSpinner;
	GtkWidget * lbPageNo;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class ToolPageLayer: public AbstractToolItem {
public:
	ToolPageLayer(ActionHandler * handler, String id, ActionType type) :
		AbstractToolItem(id, handler, type, NULL) {
		layerComboBox = gtk_combo_box_new_text();
		layerCount = -5;
		inCbUpdate = false;

		g_signal_connect(layerComboBox, "changed", G_CALLBACK(&cbSelectCallback), this);
	}

	virtual ~ToolPageLayer() {
	}

	static void cbSelectCallback(GtkComboBox *widget, ToolPageLayer * tpl) {
		if (tpl->inCbUpdate) {
			return;
		}

		GtkComboBox * cb = GTK_COMBO_BOX(tpl->layerComboBox);
		int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(cb), NULL);
		int selected = count - gtk_combo_box_get_active(widget) - 1;

		tpl->handler->actionPerformed(ACTION_FOOTER_LAYER, GROUP_NOGROUP, NULL, NULL, NULL, true);
	}

	int getSelectedLayer() {
		GtkComboBox * cb = GTK_COMBO_BOX(layerComboBox);
		int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(cb), NULL);
		int selected = count - gtk_combo_box_get_active(cb) - 1;
		return selected;
	}

	void setSelectedLayer(int selected) {
		GtkComboBox * cb = GTK_COMBO_BOX(layerComboBox);
		int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(cb), NULL);

		gtk_combo_box_set_active(cb, count - selected - 1);
	}

	void setLayerCount(int layer, int selected) {
		inCbUpdate = true;

		int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(GTK_COMBO_BOX(layerComboBox)), NULL);

		for (int i = count - 1; i >= 0; i--) {
			gtk_combo_box_remove_text(GTK_COMBO_BOX(layerComboBox), i);
		}

		gtk_combo_box_append_text(GTK_COMBO_BOX(layerComboBox), "Background");
		for (int i = 1; i <= layer; i++) {
			char * text = g_strdup_printf(_("Layer %i"), i);
			gtk_combo_box_prepend_text(GTK_COMBO_BOX(layerComboBox), text);
			g_free(text);
		}

		setSelectedLayer(selected);

		this->layerCount = layer;
		inCbUpdate = false;
	}

protected:
	virtual GtkToolItem * newItem() {
		GtkToolItem * it = gtk_tool_item_new();

		GtkWidget * hbox = gtk_hbox_new(false, 1);
		gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_("Layer")), false, false, 7);

		gtk_box_pack_start(GTK_BOX(hbox), layerComboBox, false, false, 0);

		gtk_container_add(GTK_CONTAINER (it), hbox);

		return it;
	}

private:
	GtkWidget * layerComboBox;

	int layerCount;

	bool inCbUpdate;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


class ToolZoomSlider: public AbstractToolItem, public ZoomListener {
public:
	ToolZoomSlider(ActionHandler * handler, String id, ActionType type, ZoomControl * zoom) :
		AbstractToolItem(id, handler, type, NULL) {
		slider = NULL;
		this->zoom = zoom;

		this->horizontal = true;
		this->fixed = NULL;

		zoom->addZoomListener(this);
	}

	virtual ~ToolZoomSlider() {
	}

	static void sliderChanged(GtkRange *range, ZoomControl * zoom) {
		zoom->setZoom(gtk_range_get_value(range));
	}

	virtual void zoomChanged(double lastZoom) {
		gtk_range_set_value(GTK_RANGE(slider), zoom->getZoom());
	}

	virtual void zoomRangeValuesChanged() {
		updateScaleMarks();
	}

	// Should be called when the window size changes
	void updateScaleMarks() {
		if (!slider) {
			return;
		}
		gtk_scale_clear_marks(GTK_SCALE(slider));
		gtk_scale_add_mark(GTK_SCALE(slider), zoom->getZoom100(), horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, NULL);
		gtk_scale_add_mark(GTK_SCALE(slider), zoom->getZoomFit(), horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, NULL);
	}

	virtual void setHorizontal(bool horizontal) {
		if (horizontal) {
			GtkAllocation alloc = { 0, 0, 120, 30 };
			gtk_widget_set_allocation(fixed, &alloc);
			gtk_widget_set_size_request(slider, 120, 25);
		} else {
			GtkAllocation alloc = { 0, 0, 30, 120 };
			gtk_widget_set_allocation(fixed, &alloc);
			gtk_widget_set_size_request(slider, 25, 120);
		}
		updateScaleMarks();
	}

	virtual GtkToolItem * createItem(bool horizontal) {
		this->horizontal = horizontal;
		GtkToolItem * item = newItem();
		g_object_ref(item);

		if (GTK_IS_TOOL_ITEM(item)) {
			gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
		}
		if (GTK_IS_TOOL_BUTTON(item) || GTK_IS_TOGGLE_TOOL_BUTTON(item)) {
			g_signal_connect(item, "clicked", G_CALLBACK(&toolButtonCallback), this);
		}
		gtk_object_ref(GTK_OBJECT(item));
		return item;

	}

protected:
	virtual void enable(bool enabled) {
		if (this->item) {
			gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
		}
		if (this->slider) {
			gtk_widget_set_sensitive(GTK_WIDGET(this->slider), enabled);
		}
	}

	virtual GtkToolItem * newItem() {
		GtkToolItem * it = gtk_tool_item_new();

		if (slider) {
			g_signal_handlers_disconnect_by_func(slider, (void*)(sliderChanged), zoom);
		}

		if (horizontal) {
			slider = gtk_hscale_new_with_range(0.3, 3, 0.1);
		} else {
			slider = gtk_vscale_new_with_range(0.3, 3, 0.1);
			gtk_range_set_inverted(GTK_RANGE(slider), true);
		}
		g_signal_connect(slider, "value-changed", G_CALLBACK(sliderChanged), zoom);
		gtk_scale_set_draw_value(GTK_SCALE(slider), false);

		fixed = gtk_fixed_new();

		GtkAllocation alloc = { 0, 0, 125, 30 };
		if (!horizontal) {
			alloc.width = 30;
			alloc.height = 125;
		}

		gtk_widget_set_allocation(fixed, &alloc);

		if (horizontal) {
			gtk_fixed_put(GTK_FIXED(fixed), slider, 0, 5);
			gtk_widget_set_size_request(slider, 120, 25);
		} else {
			gtk_fixed_put(GTK_FIXED(fixed), slider, 5, 0);
			gtk_widget_set_size_request(slider, 25, 120);
		}

		gtk_container_add(GTK_CONTAINER (it), fixed);
		gtk_range_set_value(GTK_RANGE(slider), zoom->getZoom());
		updateScaleMarks();

		return it;
	}

private:
	GtkWidget * slider;
	GtkWidget * fixed;
	ZoomControl * zoom;
	bool horizontal;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


class FontButton: public AbstractToolItem {
public:
	FontButton(ActionHandler * handler, GladeGui *gui, String id, ActionType type, String description,
			GtkWidget * menuitem = NULL) :
		AbstractToolItem(id, handler, type, menuitem) {
		this->gui = gui;
		this->description = description;
		fontButton = NULL;
	}

	virtual void activated(GdkEvent *event, GtkMenuItem *menuitem, GtkToolButton *toolbutton) {
		GtkFontButton * button = GTK_FONT_BUTTON(fontButton);

		String name = gtk_font_button_get_font_name(button);

		int pos = name.lastIndexOf(" ");
		this->font.setName(name.substring(0, pos));
		this->font.setSize(atof(name.substring(pos+1).c_str()));

		handler->actionPerformed(ACTION_SELECT_FONT, GROUP_NOGROUP, event, menuitem, NULL, true);
	}

	virtual ~FontButton() {
	}


	void setFont(XojFont & font) {
		this->font = font;
		GtkFontButton * button = GTK_FONT_BUTTON(fontButton);

		String txt = font.getName();
		txt += " ";
		txt += font.getSize();

		gtk_font_button_set_font_name(button, txt.c_str());
	}

	XojFont getFont() {
		return font;
	}

protected:
	virtual GtkToolItem * createItem(bool horizontal) {
		if (item) {
			return item;
		}

		item = newItem();
		g_object_ref(item);
		gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
		gtk_object_ref(GTK_OBJECT(item));
		g_signal_connect(fontButton, "font_set", G_CALLBACK(&toolButtonCallback), this);
		return item;
	}

	virtual GtkToolItem * newItem() {
		if (fontButton) {
			g_object_unref(fontButton);
		}
		GtkToolItem * it;

		it = gtk_tool_item_new();

		fontButton = gtk_font_button_new();
		gtk_widget_show(fontButton);
		gtk_container_add(GTK_CONTAINER (it), fontButton);
		gtk_font_button_set_use_font(GTK_FONT_BUTTON (fontButton), TRUE);
		gtk_button_set_focus_on_click(GTK_BUTTON (fontButton), FALSE);

		gtk_tool_item_set_tooltip_text(it, description.c_str());

		return it;
	}

private:
	GtkWidget * fontButton;
	GladeGui * gui;
	String description;

	XojFont font;
};

#endif /* __TOOLITEMS_H__ */
