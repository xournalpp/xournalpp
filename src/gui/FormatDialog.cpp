#include "FormatDialog.h"
#include "../gettext.h"

#include "../model/FormatDefinitions.h"

FormatDialog::FormatDialog(Settings * settings, double width, double heigth) :
	GladeGui("pagesize.glade", "pagesizeDialog") {
	this->orientation = ORIENTATION_NOT_DEFINED;
	this->selectedScale = 0;
	this->settings = settings;

	SElement & format = settings->getCustomElement("format");
	String unit;

	if (format.getString("unit", unit)) {
		for (int i = 0; i < XOJ_UNIT_COUNT; i++) {
			if (unit == XOJ_UNITS[i].name) {
				this->selectedScale = i;
				break;
			}
		}
	}

	this->scale = XOJ_UNITS[this->selectedScale].scale;
	this->origHeight = heigth;
	this->origWidth = width;

	this->width = -1;
	this->height = -1;

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spinWidth")), this->origWidth / this->scale);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spinHeight")), this->origHeight / this->scale);

	GtkWidget * cbUnit = get("cbUnit");
	GtkListStore * store = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_combo_box_set_model(GTK_COMBO_BOX(cbUnit), GTK_TREE_MODEL(store));
	g_object_unref(store);

	GtkCellRenderer * cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT (cbUnit), cell, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT (cbUnit), cell, "text", 0, NULL);

	for (int i = 0; i < XOJ_UNIT_COUNT; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(cbUnit), XOJ_UNITS[i].name);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbUnit), this->selectedScale);

	GtkWidget * cbTemplate = get("cbTemplate");
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_combo_box_set_model(GTK_COMBO_BOX(cbTemplate), GTK_TREE_MODEL(store));
	g_object_unref(store);

	cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT (cbTemplate), cell, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT (cbTemplate), cell, "text", 0, NULL);

	int selectedFormat = -1;

	String formatlist = settings->getVisiblePageFormats();

	if (heigth < width) {
		double tmp = width;
		width = heigth;
		heigth = tmp;
	}

	this->list = gtk_paper_size_get_paper_sizes(false);
	int i = 0;
	GList * next = NULL;
	for (GList * l = list; l != NULL; l = next) {
		GtkPaperSize * s = (GtkPaperSize *) l->data;
		next = l->next;

		double w = gtk_paper_size_get_width(s, GTK_UNIT_POINTS);
		double h = gtk_paper_size_get_height(s, GTK_UNIT_POINTS);

		bool visible = false;

		if (((int) (w - width) * 10) == 0 && ((int) (h - heigth) * 10) == 0) {
			selectedFormat = i;
			visible = true;
		}

		if (formatlist.indexOf(gtk_paper_size_get_name(s)) != -1) {
			visible = true;
		}

		if (visible) {
			GtkTreeIter iter;
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter, 0, gtk_paper_size_get_display_name(s), -1);
			gtk_list_store_set(store, &iter, 1, s, -1);
			i++;
		} else {
			gtk_paper_size_free(s);
			this->list = g_list_remove_link(this->list, l);
		}
	}

	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, _("Custom"), -1);
	gtk_list_store_set(store, &iter, 1, NULL, -1);

	// not found, select custom format
	if (selectedFormat == -1) {
		selectedFormat = i;
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(cbTemplate), selectedFormat);

	spinValueChangedCb(NULL, this);

	g_signal_connect(get("btLandscape"), "toggled", G_CALLBACK(landscapeSelectedCb), this);
	g_signal_connect(get("btPortrait"), "toggled", G_CALLBACK(portraitSelectedCb), this);
	g_signal_connect(cbTemplate, "changed", G_CALLBACK(cbFormatChangedCb), this);
	g_signal_connect(cbUnit, "changed", G_CALLBACK(cbUnitChanged), this);

	g_signal_connect(get("spinWidth"), "value-changed", G_CALLBACK(spinValueChangedCb), this);
	g_signal_connect(get("spinHeight"), "value-changed", G_CALLBACK(spinValueChangedCb), this);
}

FormatDialog::~FormatDialog() {
	for (GList * l = this->list; l != NULL; l = l->next) {
		if (l->data) {
			GtkPaperSize * s = (GtkPaperSize *) l->data;
			gtk_paper_size_free(s);
		}
	}

	g_list_free(this->list);
}

double FormatDialog::getWidth() {
	return this->width;
}

double FormatDialog::getHeight() {
	return this->height;
}

void FormatDialog::setOrientation(Orientation orientation) {
	if (this->orientation == orientation) {
		return;
	}
	this->orientation = orientation;

	GtkWidget * btPortrait = get("btPortrait");
	GtkWidget * btLandscape = get("btLandscape");

	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(btPortrait), orientation == ORIENTATION_PORTRAIT);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(btLandscape), orientation == ORIENTATION_LANDSCAPE);
}

void FormatDialog::spinValueChangedCb(GtkSpinButton * spinbutton, FormatDialog * dlg) {
	double width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinWidth"))) * dlg->scale;
	double height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinHeight"))) * dlg->scale;

	if (width < height) {
		dlg->setOrientation(ORIENTATION_PORTRAIT);
	} else if (width > height) {
		dlg->setOrientation(ORIENTATION_LANDSCAPE);
	} else {
		dlg->setOrientation(ORIENTATION_NOT_DEFINED);
	}

	int i = 0;
	for (GList * l = dlg->list; l != NULL; l = l->next) {
		GtkPaperSize * s = (GtkPaperSize *) l->data;
		double w = gtk_paper_size_get_width(s, GTK_UNIT_POINTS);
		double h = gtk_paper_size_get_height(s, GTK_UNIT_POINTS);

		if (((int) (w - width) * 10) == 0 && ((int) (h - height) * 10) == 0) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(dlg->get("cbTemplate")), i);
			return;
		}
		i++;
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(dlg->get("cbTemplate")), i);
}

void FormatDialog::cbUnitChanged(GtkComboBox * widget, FormatDialog * dlg) {
	int selectd = gtk_combo_box_get_active(widget);
	if (dlg->selectedScale == selectd) {
		return;
	}

	double width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")));
	double height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")));

	width *= dlg->scale;
	height *= dlg->scale;

	dlg->selectedScale = selectd;
	dlg->scale = XOJ_UNITS[dlg->selectedScale].scale;

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")), width / dlg->scale);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")), height / dlg->scale);
}

void FormatDialog::cbFormatChangedCb(GtkComboBox * widget, FormatDialog * dlg) {
	GtkTreeIter iter;

	if (gtk_combo_box_get_active_iter(widget, &iter)) {
		GtkTreeModel * model = gtk_combo_box_get_model(widget);

		GValue value = { 0 };
		gtk_tree_model_get_value(model, &iter, 1, &value);

		if (G_VALUE_HOLDS_POINTER(&value)) {
			GtkPaperSize * s = (GtkPaperSize *) g_value_get_pointer(&value);

			if (s == NULL) {
				return;
			}

			double width = gtk_paper_size_get_width(s, GTK_UNIT_POINTS) / dlg->scale;
			double height = gtk_paper_size_get_height(s, GTK_UNIT_POINTS) / dlg->scale;

			if (dlg->orientation == ORIENTATION_LANDSCAPE) {
				double tmp = width;
				width = height;
				height = tmp;
			} else {
				dlg->setOrientation(ORIENTATION_PORTRAIT);
			}

			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")), width);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")), height);
		}
	}
}

void FormatDialog::portraitSelectedCb(GtkToggleToolButton * bt, FormatDialog * dlg) {
	bool activated = gtk_toggle_tool_button_get_active(bt);

	if (activated) {
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(dlg->get("btLandscape")), false);
		dlg->orientation = ORIENTATION_PORTRAIT;

		double width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")));
		double height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")));

		if (width > height) {
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")), height);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")), width);
		}
	}
}

void FormatDialog::landscapeSelectedCb(GtkToggleToolButton * bt, FormatDialog * dlg) {
	bool activated = gtk_toggle_tool_button_get_active(bt);

	if (activated) {
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(dlg->get("btPortrait")), false);
		dlg->orientation = ORIENTATION_LANDSCAPE;

		double width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")));
		double height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")));

		if (width < height) {
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")), height);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")), width);
		}
	}
}

void FormatDialog::show() {
	int ret = 0;
	while (ret == 0) {
		ret = gtk_dialog_run(GTK_DIALOG(this->window));
		if (ret == 0) {
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spinWidth")), this->origWidth / this->scale);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spinHeight")), this->origHeight / this->scale);
		}
	}

	if (ret == 1) { //OK
		SElement & format = settings->getCustomElement("format");
		format.setString("unit", XOJ_UNITS[this->selectedScale].name);
		settings->customSettingsChanged();

		this->width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spinWidth"))) * this->scale;
		this->height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spinHeight"))) * this->scale;
	}

	gtk_widget_hide(this->window);
}
