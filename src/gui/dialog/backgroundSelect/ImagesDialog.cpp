#include "ImagesDialog.h"

#include "ImageElementView.h"

#include "model/BackgroundImage.h"
#include "model/Document.h"


ImagesDialog::ImagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings)
 : BackgroundSelectDialogBase(gladeSearchPath, doc, settings, "images.glade", "ImagesDialog")
{
	loadImagesFromPages();

	g_signal_connect(get("buttonOk"), "clicked", G_CALLBACK(okButtonCallback), this);
	g_signal_connect(get("btFilechooser"), "clicked", G_CALLBACK(filechooserButtonCallback), this);
}

ImagesDialog::~ImagesDialog()
{
}

void ImagesDialog::loadImagesFromPages()
{
	for (size_t i = 0; i < doc->getPageCount(); i++)
	{
		PageRef p = doc->getPage(i);

		if (!p->getBackgroundType().isImagePage())
		{
			continue;
		}

		if (p->getBackgroundImage().isEmpty())
		{
			continue;
		}

		if (isImageAlreadyInTheList(p->getBackgroundImage()))
		{
			// Do not display the same image twice
			continue;
		}

		ImageElementView* iv = new ImageElementView(this->elements.size(), this);
		iv->backgroundImage = p->getBackgroundImage();
		this->elements.push_back(iv);
	}
}

bool ImagesDialog::isImageAlreadyInTheList(BackgroundImage& image)
{
	for (BaseElementView* v : this->elements)
	{
		ImageElementView* iv = (ImageElementView*)v;
		if (iv->backgroundImage == image)
		{
			return true;
		}
	}

	return false;
}

void ImagesDialog::okButtonCallback(GtkButton* button, ImagesDialog* dlg)
{
	dlg->confirmed = true;
	gtk_widget_hide(dlg->window);
}

void ImagesDialog::filechooserButtonCallback(GtkButton* button, ImagesDialog* dlg)
{
	dlg->selected = -2;
	dlg->confirmed = true;
	gtk_widget_hide(dlg->window);
}

bool ImagesDialog::shouldShowFilechooser()
{
	return selected == -2 && confirmed;
}

BackgroundImage ImagesDialog::getSelectedImage()
{
	if (confirmed && selected >= 0 && selected < (int)elements.size())
	{
		return ((ImageElementView*)elements[selected])->backgroundImage;
	}
	else
	{
		return BackgroundImage();
	}
}

void ImagesDialog::show(GtkWindow* parent)
{
	if (this->elements.empty())
	{
		this->selected = -2;
		this->confirmed = true;
	}
	else
	{
		BackgroundSelectDialogBase::show(parent);
	}
}
