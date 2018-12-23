#include "PageBackgroundChangeController.h"

#include "control/Control.h"
#include "control/pagetype/PageTypeHandler.h"
#include "gui/dialog/backgroundSelect/ImagesDialog.h"
#include "gui/dialog/backgroundSelect/PdfPagesDialog.h"
#include "stockdlg/ImageOpenDlg.h"
#include "undo/GroupUndoAction.h"
#include "undo/PageBackgroundChangedUndoAction.h"

#include <Util.h>
#include <i18n.h>


PageBackgroundChangeController::PageBackgroundChangeController(Control* control)
 : control(control),
   currentPageType(new PageTypeMenu(control->getPageTypes(), control->getSettings(), false, true)),
   ignoreEvent(false)
{
	XOJ_INIT_TYPE(PageBackgroundChangeController);

	currentPageType->setListener(this);

	currentPageType->hideCopyPage();

	currentPageType->addApplyBackgroundButton(control->getPageBackgroundChangeController(), true);

	registerListener(control);
}

PageBackgroundChangeController::~PageBackgroundChangeController()
{
	XOJ_CHECK_TYPE(PageBackgroundChangeController);

	delete this->currentPageType;
	this->currentPageType = NULL;

	XOJ_RELEASE_TYPE(PageBackgroundChangeController);
}

GtkWidget* PageBackgroundChangeController::getMenu()
{
	XOJ_CHECK_TYPE(PageBackgroundChangeController);

	return currentPageType->getMenu();
}

void PageBackgroundChangeController::changeAllPagesBackground(PageType pt)
{
	XOJ_CHECK_TYPE(PageBackgroundChangeController);

	control->clearSelectionEndText();

	Document* doc = control->getDocument();

	GroupUndoAction* groupUndoAction = new GroupUndoAction();

	for (size_t p = 0; p < doc->getPageCount(); p++)
	{
		PageRef page = doc->getPage(p);
		if (!page.isValid())
		{
			// Should not happen
			continue;
		}

		// Get values for Undo / Redo
		double origW = page->getWidth();
		double origH = page->getHeight();
		BackgroundImage origBackgroundImage = page->getBackgroundImage();
		int origPdfPage = page->getPdfPageNr();
		PageType origType = page->getBackgroundType();

		// Apply the new background
		applyPageBackground(page, pt);

		control->firePageChanged(p);
		control->updateBackgroundSizeButton();

		UndoAction* undo = new PageBackgroundChangedUndoAction(page, origType, origPdfPage, origBackgroundImage, origW, origH);
		groupUndoAction->addAction(undo);
	}

	control->getUndoRedoHandler()->addUndoAction(groupUndoAction);
}

void PageBackgroundChangeController::changeCurrentPageBackground(PageTypeInfo* info)
{
	XOJ_CHECK_TYPE(PageBackgroundChangeController);

	if (ignoreEvent)
	{
		return;
	}

	control->clearSelectionEndText();

	PageRef page = control->getCurrentPage();
	if (!page.isValid())
	{
		return;
	}

	Document* doc = control->getDocument();
	size_t pageNr = doc->indexOf(page);
	if (pageNr == size_t_npos)
	{
		return; // should not happen...
	}

	// Get values for Undo / Redo
	double origW = page->getWidth();
	double origH = page->getHeight();
	BackgroundImage origBackgroundImage = page->getBackgroundImage();
	int origPdfPage = page->getPdfPageNr();
	PageType origType = page->getBackgroundType();

	// Apply the new background
	applyPageBackground(page, info->page);

	control->firePageChanged(pageNr);
	control->updateBackgroundSizeButton();

	UndoAction* undo = new PageBackgroundChangedUndoAction(page, origType, origPdfPage, origBackgroundImage, origW, origH);
	control->getUndoRedoHandler()->addUndoAction(undo);
}

/**
 * Apply a new Image Background, asks the user which image should be inserted
 *
 * @return true on success, false if the user cancels
 */
bool PageBackgroundChangeController::applyImageBackground(PageRef page)
{
	XOJ_CHECK_TYPE(PageBackgroundChangeController);

	Document* doc = control->getDocument();

	doc->lock();
	ImagesDialog dlg(control->getGladeSearchPath(), doc, control->getSettings());
	doc->unlock();

	dlg.show(GTK_WINDOW(control->getGtkWindow()));
	BackgroundImage img = dlg.getSelectedImage();

	if (!img.isEmpty())
	{
		page->setBackgroundImage(img);
		page->setBackgroundType(PageType(":image"));
	}
	else if (dlg.shouldShowFilechooser())
	{
		bool attach = false;
		GFile* file = ImageOpenDlg::show(control->getGtkWindow(), control->getSettings(), true, &attach);
		string filename;
		if (file == NULL)
		{
			// The user canceled
			return false;
		}
		else
		{
			char* name = g_file_get_path(file);
			filename = name;
			g_free(name);
			name = NULL;
			g_object_unref(file);
			file = NULL;
		}

		BackgroundImage newImg;
		GError* err = NULL;
		newImg.loadFile(filename, &err);
		newImg.setAttach(attach);
		if (err)
		{
			Util::showErrorToUser(control->getGtkWindow(), FS(_F("This image could not be loaded. Error message: {1}") % err->message));
			g_error_free(err);
			return false;
		}
		else
		{
			page->setBackgroundImage(newImg);
			page->setBackgroundType(PageType(":image"));
		}
	}

	// Apply correct page size
	GdkPixbuf* pixbuf = page->getBackgroundImage().getPixbuf();
	if (pixbuf)
	{
		page->setSize(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));

		size_t pageNr = doc->indexOf(page);
		if (pageNr != size_t_npos)
		{
			// Only if the page is already inserted into the document
			control->firePageSizeChanged(pageNr);
		}
	}

	return true;
}

/**
 * Apply a new PDF Background, asks the user which page should be selected
 *
 * @return true on success, false if the user cancels
 */
bool PageBackgroundChangeController::applyPdfBackground(PageRef page)
{
	XOJ_CHECK_TYPE(PageBackgroundChangeController);

	Document* doc = control->getDocument();

	if (doc->getPdfPageCount() == 0)
	{

		string msg = _("You don't have any PDF pages to select from. Cancel operation.\n"
					   "Please select another background type: Menu \"Journal\" → \"Configure Page Template\".");
		Util::showErrorToUser(control->getGtkWindow(), msg);
		return false;
	}

	doc->lock();
	PdfPagesDialog* dlg = new PdfPagesDialog(control->getGladeSearchPath(), doc, control->getSettings());
	doc->unlock();

	dlg->show(GTK_WINDOW(control->getGtkWindow()));

	int selected = dlg->getSelectedPage();
	delete dlg;

	if (selected >= 0 && selected < (int) doc->getPdfPageCount())
	{
		// no need to set a type, if we set the page number the type is also set
		page->setBackgroundPdfPageNr(selected);

		XojPdfPageSPtr p = doc->getPdfPage(selected);
		page->setSize(p->getWidth(), p->getHeight());
	}

	return true;
}

/**
 * Apply the background to the page, asks for PDF Page or Image, if needed
 *
 * @return true on success, false if the user cancels
 */
bool PageBackgroundChangeController::applyPageBackground(PageRef page, PageType pt)
{
	XOJ_CHECK_TYPE(PageBackgroundChangeController);

	if (pt.isPdfPage())
	{
		return applyPdfBackground(page);
	}
	else if (pt.isImagePage())
	{
		return applyImageBackground(page);
	}
	else
	{
		page->setBackgroundType(pt);
		return true;
	}
}

/**
 * Copy the background from source to target
 */
void PageBackgroundChangeController::copyBackgroundFromOtherPage(PageRef target, PageRef source)
{
	XOJ_CHECK_TYPE(PageBackgroundChangeController);

	// Copy page size
	target->setSize(source->getWidth(), source->getHeight());

	// Copy page background type
	PageType bg = source->getBackgroundType();
	target->setBackgroundType(bg);

	if (bg.isPdfPage())
	{
		// If PDF: Copy PDF Page
		target->setBackgroundPdfPageNr(source->getPdfPageNr());
	}
	else if (bg.isImagePage())
	{
		// If Image: Copy the Image
		target->setBackgroundImage(source->getBackgroundImage());
	}
	else
	{
		// Copy the background color
		target->setBackgroundColor(source->getBackgroundColor());
	}
}

void PageBackgroundChangeController::insertNewPage(size_t position)
{
	XOJ_CHECK_TYPE(PageBackgroundChangeController);

	control->clearSelectionEndText();

	Document* doc = control->getDocument();
	if (position > doc->getPageCount())
	{
		position = doc->getPageCount();
	}

	PageTemplateSettings model;
	model.parse(control->getSettings()->getPageTemplate());

	PageRef page = new XojPage(model.getPageWidth(), model.getPageHeight());

	PageType pt = control->getNewPageType()->getSelected();
	PageRef current = control->getCurrentPage();

	// current.isValid() should always be true, but if you open an invalid file or something like this...
	if (pt.format == ":copy" && current.isValid())
	{
		copyBackgroundFromOtherPage(page, current);
	}
	else
	{
		// Create a new page from template
		if (!applyPageBackground(page, pt))
		{
			// User canceled PDF or Image Selection
			return;
		}

		// Set background Color
		page->setBackgroundColor(model.getBackgroundColor());

		if (model.isCopyLastPageSize() && current.isValid())
		{
			page->setSize(current->getWidth(), current->getHeight());
		}
	}

	control->insertPage(page, position);
}

void PageBackgroundChangeController::documentChanged(DocumentChangeType type)
{
}

void PageBackgroundChangeController::pageSizeChanged(size_t page)
{
}

void PageBackgroundChangeController::pageChanged(size_t page)
{
}

void PageBackgroundChangeController::pageInserted(size_t page)
{
}

void PageBackgroundChangeController::pageDeleted(size_t page)
{
}

void PageBackgroundChangeController::pageSelected(size_t page)
{
	PageRef current = control->getCurrentPage();
	if (!current.isValid())
	{
		return;
	}

	ignoreEvent = true;
	currentPageType->setSelected(current->getBackgroundType());
	ignoreEvent = false;
}

void PageBackgroundChangeController::applyCurrentPageBackground(bool allPages)
{
	PageType pt = control->getNewPageType()->getSelected();

	if (allPages)
	{
		changeAllPagesBackground(pt);
	}
	else
	{
		PageTypeInfo info;
		info.page = pt;
		changeCurrentPageBackground(&info);
	}
}


