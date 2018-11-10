#include "ImagesDialog.h"

#include "ImageElementView.h"

#include "model/BackgroundImage.h"
//#include "model/Document.h"

//#include <config.h>
//#include <Util.h>

//#include <math.h>

ImagesDialog::ImagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings)
 : BackgroundSelectDialogBase(gladeSearchPath, doc, settings, "images.glade", "ImagesDialog")
{
	XOJ_INIT_TYPE(ImagesDialog);

	for(int i = 0; i < 20; i++)
	{
		elements.push_back(new ImageElementView());
	}





/*
	this->backgroundInitialized = false;
	this->selected = -1;
	this->lastWidth = -1;
	this->selectedPage = -1;

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollPreview), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollPreview), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(scrollPreview), widget);
	gtk_box_pack_start(GTK_BOX(get("vbox")), scrollPreview, true, true, 0);

	g_signal_connect(this->window, "size-allocate", G_CALLBACK(sizeAllocate), this);

	int x = 0;
	for (size_t i = 0; i < doc->getPageCount(); i++)
	{
		PageRef p = doc->getPage(i);

		if (p->getBackgroundType() == BACKGROUND_TYPE_IMAGE)
		{
			if (p->getBackgroundImage().isEmpty())
			{
				continue;
			}

			bool found = false;

			for (ImageView* v : this->images)
			{
				if (v->backgroundImage == p->getBackgroundImage())
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				ImageView* page = new ImageView(x, this);
				page->backgroundImage = p->getBackgroundImage();
				page->updateSize();
				gtk_layout_put(GTK_LAYOUT(this->widget), page->getWidget(), 0, 0);

				this->images.push_back(page);
				x++;
			}
		}
	}

	if (!this->images.empty())
	{
		setSelected(0);
	}

	gtk_widget_show_all(this->scrollPreview);

	layout();
	updateOkButton();

	g_signal_connect(get("buttonOk"), "clicked", G_CALLBACK(okButtonCallback), this);
	g_signal_connect(get("btFilechooser"), "clicked", G_CALLBACK(filechooserButtonCallback), this);
	*/
}

ImagesDialog::~ImagesDialog()
{
	XOJ_CHECK_TYPE(ImagesDialog);
/*
	for (ImageView* view : this->images)
	{
		delete view;
	}
	this->images.clear();
*/
	XOJ_RELEASE_TYPE(ImagesDialog);
}

/*
void ImagesDialog::updateOkButton()
{
	XOJ_CHECK_TYPE(ImagesDialog);

	ImageView* p = (this->images.size() > this->selected ? this->images[this->selected] : NULL);
	gtk_widget_set_sensitive(get("buttonOk"), p && gtk_widget_get_visible(p->getWidget()));
}

void ImagesDialog::okButtonCallback(GtkButton* button, ImagesDialog* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, ImagesDialog);

	dlg->selectedPage = dlg->selected;
}

void ImagesDialog::filechooserButtonCallback(GtkButton* button, ImagesDialog* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, ImagesDialog);

	dlg->selectedPage = -2;
	gtk_widget_hide(dlg->window);
}

bool ImagesDialog::shouldShowFilechooser()
{
	XOJ_CHECK_TYPE(ImagesDialog);

	return this->selectedPage == -2;
}
*/
bool ImagesDialog::shouldShowFilechooser()
{
	return true;
}
BackgroundImage ImagesDialog::getSelectedImage()
{
	XOJ_CHECK_TYPE(ImagesDialog);
/*
	if (this->images.size() > this->selected)
	{
		return this->images[this->selected]->backgroundImage;
	}
	else
	{
		return BackgroundImage();
	}
	*/
	return BackgroundImage();
}
/*
void ImagesDialog::sizeAllocate(GtkWidget* widget, GtkRequisition* requisition, ImagesDialog* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, ImagesDialog);

	GtkAllocation alloc = {0};
	gtk_widget_get_allocation(dlg->scrollPreview, &alloc);
	if (dlg->lastWidth == alloc.width)
	{
		return;
	}
	dlg->lastWidth = alloc.width;
	dlg->layout();
}

void ImagesDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(ImagesDialog);

	if (this->images.empty())
	{
		this->selectedPage = -2;
	}
	else
	{
		gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
		gtk_dialog_run(GTK_DIALOG(this->window));
		gtk_widget_hide(this->window);
	}
}



void ImagesDialog::setSelected(size_t selected)
{
	XOJ_CHECK_TYPE(ImagesDialog);

	if (this->selected == selected)
	{
		return;
	}

	int lastSelected = this->selected;
	ImageView* p = this->images[selected];
	if (p)
	{
		p->setSelected(true);
		this->selected = selected;
	}
	p = this->images[lastSelected];
	if (p)
	{
		p->setSelected(false);
	}

	updateOkButton();
}

*/
