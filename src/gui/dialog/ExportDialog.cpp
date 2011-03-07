#include "ExportDialog.h"
#include "../../util/PageRange.h"

#include <config.h>
#include <glib/gi18n-lib.h>

ExportDialog::ExportDialog(GladeSearchpath * gladeSearchPath, Settings * settings, int pageCount, int currentPage) :
	GladeGui(gladeSearchPath, "export.glade", "exportDialog") {
	this->range = NULL;
	this->pageCount = pageCount;
	this->currentPage = currentPage;
	this->resolution = 72;
	this->settings = settings;
	this->type = EXPORT_FORMAT_PNG;

	GtkFileChooser * chooser = GTK_FILE_CHOOSER(get("fcOutput"));
	gtk_file_chooser_set_local_only(chooser, true);
	gtk_file_chooser_set_current_folder(chooser, settings->getLastSavePath().c_str());
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

	GtkFileChooser * chooser = GTK_FILE_CHOOSER(get("fcOutput"));
	char * folder = gtk_file_chooser_get_current_folder(chooser);
	this->settings->setLastSavePath(folder);
	g_free(folder);
}

ExportFormtType ExportDialog::getFormatType() {
	return this->type;
}

int ExportDialog::getPngDpi() {
	return this->resolution;
}

String ExportDialog::getFolder() {
	GtkFileChooser * chooser = GTK_FILE_CHOOSER(get("fcOutput"));
	char * folder = gtk_file_chooser_get_current_folder(chooser);
	String f = folder;
	g_free(folder);
	return f;
}

String ExportDialog::getFilename() {
	return gtk_entry_get_text(GTK_ENTRY(get("txtFilename")));
}

bool ExportDialog::validate() {

	if (gtk_entry_get_text_length(GTK_ENTRY(get("txtFilename"))) == 0) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *this, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				_("The filename should not be empty"));

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return false;
	}

	return true;
}

void ExportDialog::show() {
	int res = 0;
	do {
		res = gtk_dialog_run(GTK_DIALOG(this->window));
		if (res == 2) {
			if (validate()) {
				handleData();
				break;
			}
		}
	} while (res == 2);

	gtk_widget_hide(this->window);
}

