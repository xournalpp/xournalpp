#include "PdfPagesDialog.h"

#include "PdfElementView.h"

#include "gui/Shadow.h"
#include "model/Document.h"

#include <Util.h>
#include <i18n.h>


PdfPagesDialog::PdfPagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings)
 : BackgroundSelectDialogBase(gladeSearchPath, doc, settings, "pdfpages.glade", "pdfPagesDialog")
{
	for (size_t i = 0; i < doc->getPdfPageCount(); i++)
	{
		XojPdfPageSPtr p = doc->getPdfPage(i);
		PdfElementView* pv = new PdfElementView(elements.size(), p, this);
		elements.push_back(pv);
	}
	if (doc->getPdfPageCount() > 0)
	{
		setSelected(0);
	}

	for (size_t i = 0; i < doc->getPageCount(); i++)
	{
		PageRef p = doc->getPage(i);

		if (p->getBackgroundType().isPdfPage())
		{
			int pdfPage = p->getPdfPageNr();
			if (pdfPage >= 0 && pdfPage < (int)elements.size())
			{
				((PdfElementView*)elements[p->getPdfPageNr()])->setUsed(true);
			}
		}
	}

	updateOkButton();

	g_signal_connect(get("cbOnlyNotUsed"), "toggled", G_CALLBACK(onlyNotUsedCallback), this);
	g_signal_connect(get("buttonOk"), "clicked", G_CALLBACK(okButtonCallback), this);
}

PdfPagesDialog::~PdfPagesDialog()
{
}

void PdfPagesDialog::updateOkButton()
{
	bool valid = false;
	if (selected >= 0 && selected < (int)elements.size())
	{
		BaseElementView* p = this->elements[this->selected];
		valid = gtk_widget_get_visible(p->getWidget());
	}

	gtk_widget_set_sensitive(get("buttonOk"), valid);
}

void PdfPagesDialog::okButtonCallback(GtkButton* button, PdfPagesDialog* dlg)
{
	dlg->confirmed = true;
}

void PdfPagesDialog::onlyNotUsedCallback(GtkToggleButton* tb, PdfPagesDialog* dlg)
{
	if (gtk_toggle_button_get_active(tb))
	{
		for (BaseElementView* p : dlg->elements)
		{
			PdfElementView* pv = (PdfElementView*)p;
			pv->setHideUnused();
		}
	}
	else
	{
		gtk_widget_show_all(dlg->scrollPreview);
	}

	dlg->layout();
	dlg->updateOkButton();
}

double PdfPagesDialog::getZoom()
{
	return 0.25;
}

int PdfPagesDialog::getSelectedPage()
{
	if (confirmed)
	{
		return this->selected;
	}

	return -1;
}

void PdfPagesDialog::show(GtkWindow* parent)
{
	GtkWidget* w = get("cbOnlyNotUsed");

	int unused = 0;
	for (BaseElementView* p : elements)
	{
		PdfElementView* pv = (PdfElementView*)p;
		if (!pv->isUsed())
		{
			unused++;
		}
	}

	gtk_button_set_label(GTK_BUTTON(w),
		(unused == 1
			? _("Show only not used pages (one unused page)")
			: FC(_F("Show only not used pages ({1} unused pages)") % unused)));

	BackgroundSelectDialogBase::show(parent);
}
