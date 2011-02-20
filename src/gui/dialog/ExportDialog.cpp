#include "ExportDialog.h"
#include "../../util/PageRange.h"

ExportDialog::ExportDialog(int pageCount, int currentPage) :
	GladeGui("export.glade", "exportDialog") {
	this->range = NULL;
	this->pageCount = pageCount;
	this->currentPage = currentPage;
	this->resolution = 72;
	this->type = EXPORT_FORMAT_PNG;

	GtkFileChooser * chooser = GTK_FILE_CHOOSER(get("fcOutput"));
	gtk_file_chooser_set_select_multiple(chooser, false);
	gtk_file_chooser_set_action(chooser, GTK_FILE_CHOOSER_ACTION_SAVE);
}

ExportDialog::~ExportDialog() {
}

GList * ExportDialog::getRange() {
	return this->range;
}

void ExportDialog::handleData() {
	GtkWidget * rdRangeAll = get("rdRangeAll");
	GtkWidget * rdRangeCurrent = get("rdRangeCurrent");
	GtkWidget * rdRangePages = get("rdRangePages");

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangePages))) {
		this->range = PageRange::parse(gtk_entry_get_text(GTK_ENTRY(get("txtPages"))));
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangeCurrent))) {
		this->range = g_list_append(this->range, new PageRangeEntry(this->currentPage, this->currentPage));
	} else {
		this->range = g_list_append(this->range, new PageRangeEntry(0, this->pageCount - 1));
	}

	this->resolution = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spPngResolution")));

	GtkWidget * rdFormatPDF = get("rdFormatPDF");
	GtkWidget * rdFormatEPS = get("rdFormatEPS");
	GtkWidget * rdFormatSVG = get("rdFormatSVG");
	GtkWidget * rdFormatPNG = get("rdFormatPNG");

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdFormatPDF))) {
		this->type = EXPORT_FORMAT_PDF;
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdFormatEPS))) {
		this->type = EXPORT_FORMAT_EPS;
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdFormatSVG))) {
		this->type = EXPORT_FORMAT_SVG;
	} else {
		this->type = EXPORT_FORMAT_PNG;
	}
}

ExportFormtType ExportDialog::getFormatType() {
	return this->type;
}

int ExportDialog::getResolution() {
	return this->resolution;
}

void ExportDialog::show() {
	if (gtk_dialog_run(GTK_DIALOG(this->window)) == 2) {
		handleData();
	}
	gtk_widget_hide(this->window);
}

