/*
 * Xournal++
 *
 * Dialog to select the background color of a Xournal page
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#include "SelectBackgroundColorDialog.h"
#include "../../control/Control.h"
#include "../widgets/SelectColor.h"

#include <config.h>
#include <glib/gi18n-lib.h>

class ColorEntry {
public:
	ColorEntry(SelectBackgroundColorDialog * dlg, int color, bool custom) {
		XOJ_INIT_TYPE(ColorEntry);
		this->dlg = dlg;
		this->color = color;
		this->custom = custom;
	}

	~ColorEntry() {
		XOJ_RELEASE_TYPE(ColorEntry);
	}

	XOJ_TYPE_ATTRIB;

	SelectBackgroundColorDialog * dlg;
	int color;
	bool custom;
};

SelectBackgroundColorDialog::SelectBackgroundColorDialog(GladeSearchpath * gladeSearchPath, Control * control) :
	GladeGui(gladeSearchPath, "page-background-color.glade", "pageBgColorDialog") {

	XOJ_INIT_TYPE(SelectBackgroundColorDialog);

	this->control = control;
	this->colors = NULL;
	this->selected = -1;
	this->colorDlg = NULL;

	ColorEntry * e = new ColorEntry(this, -1, true);
	this->colors = g_list_append(this->colors, e);

	int predef_bgcolors_rgba[] = { 0xffffff, 0xa0e8ff, 0x80ffc0, 0xffc0d4, 0xffc080, 0xffff80 };

	GtkWidget * toolbar = get("tbPredefinedColors");

	for (int i = 0; i < sizeof(predef_bgcolors_rgba) / sizeof(int); i++) {
		int color = predef_bgcolors_rgba[i];
		ColorEntry * e = new ColorEntry(this, color, false);
		this->colors = g_list_append(this->colors, e);

		GtkWidget * iconWidget = selectcolor_new(color);
		selectcolor_set_size(iconWidget, 32);
		selectcolor_set_circle(iconWidget, true);
		GtkToolItem * it = gtk_tool_button_new(iconWidget, "");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(it), -1);
		g_signal_connect(it, "clicked", G_CALLBACK(&buttonSelectedCallback), e);
	}

	gtk_widget_show_all(toolbar);

	toolbar = get("tbLastUsedColors");

	Settings * settings = control->getSettings();
	SElement & el = settings->getCustomElement("lastUsedPageBgColor");

	int count = 0;
	el.getInt("count", count);

	for (int i = 0; i < count; i++) {
		int color = -1;
		char * settingName = g_strdup_printf("color%02i", i);
		bool read = el.getInt(settingName, color);
		g_free(settingName);

		if (!read) {
			continue;
		}

		ColorEntry * e = new ColorEntry(this, color, true);
		this->colors = g_list_append(this->colors, e);

		GtkWidget * iconWidget = selectcolor_new(color);
		selectcolor_set_size(iconWidget, 32);
		selectcolor_set_circle(iconWidget, true);
		GtkToolItem * it = gtk_tool_button_new(iconWidget, "");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(it), -1);
		g_signal_connect(it, "clicked", G_CALLBACK(&buttonSelectedCallback), e);
	}
	gtk_widget_show_all(toolbar);

	if (count == 0) {
		// no colors => not title
		GtkWidget * w = get("lbLastUsed");
		gtk_widget_hide(w);
	}

	g_signal_connect(get("cbSelect"), "clicked", G_CALLBACK(&buttonCustomCallback), this);
}

SelectBackgroundColorDialog::~SelectBackgroundColorDialog() {
	XOJ_CHECK_TYPE(SelectBackgroundColorDialog);

	for (GList * l = this->colors; l != NULL; l = l->next) {
		ColorEntry * e = (ColorEntry *) l->data;
		delete e;
	}

	XOJ_RELEASE_TYPE(SelectBackgroundColorDialog);
}

void SelectBackgroundColorDialog::showColorchooser() {
	XOJ_CHECK_TYPE(SelectBackgroundColorDialog);

	this->colorDlg = gtk_color_selection_dialog_new(_("Select color"));
	g_signal_connect(G_OBJECT
			(GTK_COLOR_SELECTION_DIALOG(this->colorDlg)->ok_button),
			"clicked", G_CALLBACK(&buttonSelectedCallback), this->colors->data); // first entry

	gtk_dialog_run(GTK_DIALOG(this->colorDlg));

	gtk_widget_destroy(this->colorDlg);
	this->colorDlg = NULL;
}

const int MAX_LAST_USED_COLORS = 8;

void SelectBackgroundColorDialog::updateLastUsedColors() {
	XOJ_CHECK_TYPE(SelectBackgroundColorDialog);

	if (this->selected < 0) {
		return;
	}

	int lastUsedColors[MAX_LAST_USED_COLORS];

	for (int i = 0; i < MAX_LAST_USED_COLORS; i++) {
		lastUsedColors[i] = -1;
	}

	Settings * settings = control->getSettings();
	SElement & el = settings->getCustomElement("lastUsedPageBgColor");

	int count = 0;
	el.getInt("count", count);

	for (int i = 0; i < count && i < MAX_LAST_USED_COLORS; i++) {
		int color = -1;
		char * settingName = g_strdup_printf("color%02i", i);
		bool read = el.getInt(settingName, color);
		g_free(settingName);

		if (read) {
			lastUsedColors[i] = color;
		}
	}

	int lastUsedColorsNew[MAX_LAST_USED_COLORS];
	int id = 0;

	lastUsedColorsNew[id++] = this->selected;

	for (int i = 0; i < MAX_LAST_USED_COLORS && id < MAX_LAST_USED_COLORS; i++) {
		int c = lastUsedColors[i];
		if (c < 0) {
			continue;
		}
		// do we already have this color in the list?

		bool found = false;

		for (int x = 0; x < id; x++) {
			if (lastUsedColorsNew[x] == c) {
				found = true;
				break;
			}
		}
		if (found) {
			continue;
		}
		lastUsedColorsNew[id++] = c;
	}

	el.setInt("count", id);
	for (int i = 0; i < id; i++) {
		char * settingName = g_strdup_printf("color%02i", i);
		el.setIntHex(settingName, lastUsedColorsNew[i]);
		g_free(settingName);
	}

	settings->customSettingsChanged();
}

void SelectBackgroundColorDialog::buttonCustomCallback(GtkButton * button, SelectBackgroundColorDialog * dlg) {
	XOJ_CHECK_TYPE_OBJ(dlg, SelectBackgroundColorDialog);

	dlg->selected = -2;
	gtk_widget_hide(dlg->window);
}

void SelectBackgroundColorDialog::buttonSelectedCallback(GtkButton * button, ColorEntry * e) {
	XOJ_CHECK_TYPE_OBJ(e, ColorEntry);

	e->dlg->selected = e->color;
	if (e->custom) {
		if (e->color == -1 && e->dlg->colorDlg) {
			GdkColor color = { 0 };

			GtkWidget* cw = gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(e->dlg->colorDlg));
			gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(cw), &color);

			e->dlg->selected = (color.red / 256) << 16 | (color.green / 256) << 8 | (color.blue / 256);
		}

		e->dlg->updateLastUsedColors();
	}

	gtk_widget_hide(e->dlg->window);
}

int SelectBackgroundColorDialog::getSelectedColor() {
	XOJ_CHECK_TYPE(SelectBackgroundColorDialog);

	return this->selected;
}

void SelectBackgroundColorDialog::show() {
	XOJ_CHECK_TYPE(SelectBackgroundColorDialog);

	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
}

