#include "PdfExportJob.h"
#include "control/Control.h"
#include "pdf/popplerdirect/PdfExport.h"

#include <i18n.h>


PdfExportJob::PdfExportJob(Control* control)
 : BaseExportJob(control, _("PDF Export"))
{
	XOJ_INIT_TYPE(PdfExportJob);
}

PdfExportJob::~PdfExportJob()
{
	XOJ_RELEASE_TYPE(PdfExportJob);
}

void PdfExportJob::addFilterToDialog()
{
	XOJ_CHECK_TYPE(PdfExportJob);

	addFileFilterToDialog(_C("PDF files"), "*.pdf");
}

void PdfExportJob::prepareSavePath(path& path)
{
	XOJ_CHECK_TYPE(PdfExportJob);

	if (path.extension() != ".pdf")
    {
		path += ".pdf";
    }
}

bool PdfExportJob::isUriValid(string& uri)
{
	XOJ_CHECK_TYPE(PdfExportJob);

	if (!BaseExportJob::isUriValid(uri))
	{
		return false;
	}

	string ext = filename.extension().string();
	if (ext != ".pdf")
	{
		string msg = _C("File name needs to end with .pdf");
		GtkWidget* dialog = gtk_message_dialog_new(control->getGtkWindow(), GTK_DIALOG_DESTROY_WITH_PARENT,
												   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg.c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), control->getGtkWindow());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return false;
	}

	return true;
}

void PdfExportJob::run(bool noThreads)
{
	XOJ_CHECK_TYPE(PdfExportJob);

	Document* doc = control->getDocument();
	doc->lock();
	PdfExport pdf(doc, control);
	doc->unlock();

	if (!pdf.createPdf(this->filename))
	{
		if (control->getWindow())
		{
			callAfterRun();
		}
		else
		{
			this->errorMsg = pdf.getLastError();
		}
	}
}

