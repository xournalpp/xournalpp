#include "PdfExportJob.h"
#include "../../pdf/PdfExport.h"
#include "../Control.h"
#include "SynchronizedProgressListener.h"

PdfExportJob::PdfExportJob(Control * control) :
	BlockingJob(control, _("PDF Export")) {

	XOJ_INIT_TYPE(PdfExportJob);
}

PdfExportJob::~PdfExportJob() {
	XOJ_RELEASE_TYPE(PdfExportJob);
}

bool PdfExportJob::showFilechooser() {
	XOJ_CHECK_TYPE(PdfExportJob);

	Settings * settings = control->getSettings();
	Document * doc = control->getDocument();

	GtkWidget * dialog = gtk_file_chooser_dialog_new(_("Export PDF"), (GtkWindow*) *control->getWindow(), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), false);

	GtkFileFilter * filterPdf = gtk_file_filter_new();
	gtk_file_filter_set_name(filterPdf, _("PDF files"));
	gtk_file_filter_add_pattern(filterPdf, "*.pdf");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);

	if (!settings->getLastSavePath().isEmpty()) {
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}

	String saveFilename = "";

	doc->lock();
	if (!doc->getFilename().isEmpty()) {
		saveFilename = control->getFilename(doc->getFilename());
	} else if (!doc->getPdfFilename().isEmpty()) {
		saveFilename = control->getFilename(doc->getPdfFilename());
		saveFilename += ".xoj";
	} else {
		time_t curtime = time(NULL);
		char stime[128];
		strftime(stime, sizeof(stime), settings->getDefaultSaveName().c_str(), localtime(&curtime));

		saveFilename = stime;

		if (saveFilename.substring(-4) == ".xoj") {
			saveFilename = saveFilename.substring(0, -4);
		}
	}
	doc->unlock();

	saveFilename += ".pdf";

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), saveFilename.c_str());
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		return false;
	}

	char * name = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));

	this->filename = name;
	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
	settings->setLastSavePath(folder);
	g_free(folder);
	g_free(name);

	gtk_widget_destroy(dialog);

	return true;
}

void PdfExportJob::afterRun() {
	XOJ_CHECK_TYPE(PdfExportJob);

	if (!this->errorMsg.isEmpty()) {
		GtkWindow * win = (GtkWindow*) *control->getWindow();
		GtkWidget * dialog = gtk_message_dialog_new(win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", this->errorMsg.c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}

void PdfExportJob::run() {
	XOJ_CHECK_TYPE(PdfExportJob);

	SynchronizedProgressListener pglistener(this->control);

	Document * doc = control->getDocument();
	doc->lock();
	PdfExport pdf(doc, &pglistener);
	doc->unlock();

	if (!pdf.createPdf(this->filename)) {
		this->errorMsg = String::format(_("Create pdf failed: %s"), pdf.getLastError().c_str());

		if (control->getWindow()) {
			callAfterRun();
		} else {
			g_error("%s", this->errorMsg.c_str());
		}
	}
}
