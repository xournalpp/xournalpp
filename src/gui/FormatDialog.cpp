#include "FormatDialog.h"
#include "../gettext.h"

#include "../model/FormatDefinitions.h"

FormatDialog::FormatDialog(Settings * settings, double width, double heigth) :
	GladeGui("pagesize.glade", "pagesizeDialog") {
	this->orientation = ORIENTATION_NOT_DEFINED;
	setOrientation(ORIENTATION_PORTRAIT);
	this->selectedScale = 0;
	this->settings = settings;

	SElement & format = settings->getElement("format");
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
	store = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_combo_box_set_model(GTK_COMBO_BOX(cbTemplate), GTK_TREE_MODEL(store));
	g_object_unref(store);

	cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT (cbTemplate), cell, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT (cbTemplate), cell, "text", 0, NULL);

	int selectedFormat = XOJ_FORMAT_CUSTOM_ID;
	for (int i = 0; i < XOJ_FORMAT_COUNT; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(cbTemplate), XOJ_FORMATS[i].name);
		if (((int) (XOJ_FORMATS[i].width - width) * 10) == 0 && ((int) (XOJ_FORMATS[i].height - heigth) * 10) == 0) {
			selectedFormat = i;
		}
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbTemplate), selectedFormat);

	g_signal_connect(get("btLandscape"), "toggled", G_CALLBACK(landscapeSelectedCb), this);
	g_signal_connect(get("btPortrait"), "toggled", G_CALLBACK(portraitSelectedCb), this);
	g_signal_connect(cbTemplate, "changed", G_CALLBACK(cbFormatChangedCb), this);
	g_signal_connect(cbUnit, "changed", G_CALLBACK(cbUnitChanged), this);

	g_signal_connect(get("spinWidth"), "value-changed", G_CALLBACK(spinValueChangedCb), this);
	g_signal_connect(get("spinHeight"), "value-changed", G_CALLBACK(spinValueChangedCb), this);
}

FormatDialog::~FormatDialog() {
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
	double width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")));
	double height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")));

	if (width < height) {
		dlg->setOrientation(ORIENTATION_PORTRAIT);
	} else if (width > height) {
		dlg->setOrientation(ORIENTATION_LANDSCAPE);
	} else {
		dlg->setOrientation(ORIENTATION_NOT_DEFINED);
	}
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
	int selected = gtk_combo_box_get_active(widget);
	if (selected != XOJ_FORMAT_CUSTOM_ID) {
		double width = XOJ_FORMATS[selected].width;
		double height = XOJ_FORMATS[selected].height;

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
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spinHeight")), this->origHeight/ this->scale);
		}
	}

	if (ret == 1) { //OK
		SElement & format = settings->getElement("format");
		format.setString("unit", XOJ_UNITS[this->selectedScale].name);
		settings->customSettingsChanged();

		this->width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spinWidth"))) * this->scale;
		this->height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spinHeight"))) * this->scale;
	}

	gtk_widget_hide(this->window);
}
