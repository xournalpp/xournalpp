#include "XournalView.h"

#include "Cursor.h"
#include "Layout.h"
#include "PageView.h"
#include "RepaintHandler.h"
#include "Shadow.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "control/settings/MetadataManager.h"
#include "model/Document.h"
#include "model/Stroke.h"
#include "pageposition/PagePositionHandler.h"
#include "undo/DeleteUndoAction.h"
#include "widgets/XournalWidget.h"

#include <Rectangle.h>
#include <Util.h>

#include <gdk/gdkkeysyms.h>

#include <math.h>

XournalView::XournalView(GtkWidget* parent, Control* control)
{
	XOJ_INIT_TYPE(XournalView);

	this->control = control;
	this->cache = new PdfCache(control->getSettings()->getPdfPageCacheSize());
	registerListener(control);

	this->widget = gtk_xournal_new(this);

	// we need to refer widget here, because wo unref it somwere twice!?
	g_object_ref(this->widget);

	gtk_table_attach_defaults(GTK_TABLE(parent), this->widget, 1, 2, 0, 1);
	gtk_widget_show(this->widget);

	this->repaintHandler = new RepaintHandler(this);
	this->pagePosition = new PagePositionHandler();

	this->viewPages = NULL;
	this->viewPagesLen = 0;
	this->margin = 75;
	this->currentPage = 0;
	this->lastSelectedPage = -1;

	control->getZoomControl()->addZoomListener(this);

	gtk_widget_set_can_default(this->widget, true);
	gtk_widget_grab_default(this->widget);

	gtk_widget_grab_focus(this->widget);

	this->cleanupTimeout = g_timeout_add_seconds(5, (GSourceFunc) clearMemoryTimer, this);
}

XournalView::~XournalView()
{
	XOJ_CHECK_TYPE(XournalView);

	g_source_remove(this->cleanupTimeout);

	for (size_t i = 0; i < this->viewPagesLen; i++)
	{
		delete this->viewPages[i];
	}
	delete[] this->viewPages;
	this->viewPagesLen = 0;
	this->viewPages = NULL;

	delete this->cache;
	this->cache = NULL;
	delete this->repaintHandler;
	this->repaintHandler = NULL;

	delete this->pagePosition;
	this->pagePosition = NULL;

	this->widget = NULL;

	XOJ_RELEASE_TYPE(XournalView);
}

gint pageViewCmpSize(PageView* a, PageView* b)
{
	return a->getLastVisibleTime() - b->getLastVisibleTime();
}

gboolean XournalView::clearMemoryTimer(XournalView* widget)
{
	XOJ_CHECK_TYPE_OBJ(widget, XournalView);

	GList* list = NULL;

	for (size_t i = 0; i < widget->viewPagesLen; i++)
	{
		PageView* v = widget->viewPages[i];
		if (v->getLastVisibleTime() > 0)
		{
			list = g_list_insert_sorted(list, v, (GCompareFunc) pageViewCmpSize);
		}
	}

	int pixel = 2884560;
	int firstPages = 4;

	int i = 0;

	for (GList* l = list; l != NULL; l = l->next)
	{
		if (firstPages)
		{
			firstPages--;
		}
		else
		{
			PageView* v = (PageView*) l->data;

			if (pixel <= 0)
			{
				v->deleteViewBuffer();
			}
			else
			{
				pixel -= v->getBufferPixels();
			}
		}
		i++;
	}

	g_list_free(list);

	// call again
	return true;
}

size_t XournalView::getCurrentPage()
{
	XOJ_CHECK_TYPE(XournalView);

	return currentPage;
}

const int scrollKeySize = 30;

bool XournalView::onKeyPressEvent(GdkEventKey* event)
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = getCurrentPage();
	if (p != size_t_npos && p < this->viewPagesLen)
	{
		PageView* v = this->viewPages[p];
		if (v->onKeyPressEvent(event))
		{
			return true;
		}
	}

	// Esc leaves fullscreen mode
	if (event->keyval == GDK_Escape || event->keyval == GDK_F11)
	{
		if (control->isFullscreen())
		{
			control->enableFullscreen(false);
			return true;
		}
	}

	// F5 starts presentation modus
	if (event->keyval == GDK_F5)
	{
		if (!control->isFullscreen())
		{
			control->enableFullscreen(true, true);
			return true;
		}
	}

	guint state = event->state & gtk_accelerator_get_default_mod_mask();

	Layout* layout = gtk_xournal_get_layout(this->widget);

	if (state & GDK_SHIFT_MASK)
	{
		GtkAllocation alloc = { 0 };
		gtk_widget_get_allocation(gtk_widget_get_parent(this->widget), &alloc);
		int windowHeight = alloc.height - scrollKeySize;

		if (event->keyval == GDK_Page_Down)
		{
			layout->scrollRelativ(0, windowHeight);
			return true;
		}
		if (event->keyval == GDK_Page_Up)
		{
			layout->scrollRelativ(0, -windowHeight);
			return true;
		}
	}
	else
	{
		if (event->keyval == GDK_Page_Down)
		{
			control->getScrollHandler()->goToNextPage();
			return true;
		}
		if (event->keyval == GDK_Page_Up)
		{
			control->getScrollHandler()->goToPreviousPage();
			return true;
		}
	}


	if (event->keyval == GDK_Up)
	{
		if (control->getSettings()->isPresentationMode())
		{
			control->getScrollHandler()->goToPreviousPage();
			return true;
		}
		else
		{
			layout->scrollRelativ(0, -scrollKeySize);
			return true;
		}
	}

	if (event->keyval == GDK_Down)
	{
		if (control->getSettings()->isPresentationMode())
		{
			control->getScrollHandler()->goToNextPage();
			return true;
		}
		else
		{
			layout->scrollRelativ(0, scrollKeySize);
			return true;
		}
	}

	if (event->keyval == GDK_Left)
	{
		if (control->getSettings()->isPresentationMode())
		{
			control->getScrollHandler()->goToPreviousPage();
			return true;
		}
		layout->scrollRelativ(-scrollKeySize, 0);
		return true;
	}

	if (event->keyval == GDK_Right)
	{
		if (control->getSettings()->isPresentationMode())
		{
			control->getScrollHandler()->goToNextPage();
			return true;
		}
		layout->scrollRelativ(scrollKeySize, 0);
		return true;
	}

	if (event->keyval == GDK_End)
	{
		control->getScrollHandler()->goToLastPage();
		return true;
	}

	if (event->keyval == GDK_Home)
	{
		control->getScrollHandler()->goToFirstPage();
		return true;
	}

	return false;
}

RepaintHandler* XournalView::getRepaintHandler()
{
	XOJ_CHECK_TYPE(XournalView);

	return this->repaintHandler;
}

bool XournalView::onKeyReleaseEvent(GdkEventKey* event)
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = getCurrentPage();
	if (p != size_t_npos && p < this->viewPagesLen)
	{
		PageView* v = this->viewPages[p];
		if (v->onKeyReleaseEvent(event))
		{
			return true;
		}
	}

	return false;
}

// send the focus back to the appropriate widget
void XournalView::requestFocus()
{
	XOJ_CHECK_TYPE(XournalView);

	gtk_widget_grab_focus(this->widget);
}

bool XournalView::searchTextOnPage(string text, size_t p, int* occures, double* top)
{
	XOJ_CHECK_TYPE(XournalView);

	if (p == size_t_npos || p >= this->viewPagesLen)
	{
		return false;
	}
	PageView* v = this->viewPages[p];

	return v->searchTextOnPage(text, occures, top);
}

void XournalView::forceUpdatePagenumbers()
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = this->currentPage;
	this->currentPage = size_t_npos;

	control->firePageSelected(p);
}

PageView* XournalView::getViewFor(size_t pageNr)
{
	XOJ_CHECK_TYPE(XournalView);

	if (pageNr == size_t_npos || pageNr >= this->viewPagesLen)
	{
		return NULL;
	}
	return this->viewPages[pageNr];
}

void XournalView::pageSelected(size_t page)
{
	XOJ_CHECK_TYPE(XournalView);

	if (this->currentPage == page && this->lastSelectedPage == page)
	{
		return;
	}

	Document* doc = control->getDocument();
	doc->lock();
	path file = doc->getEvMetadataFilename();
	doc->unlock();

	control->getMetadataManager()->setInt(file, "page", page);

	if (this->lastSelectedPage != size_t_npos && this->lastSelectedPage < this->viewPagesLen)
	{
		this->viewPages[this->lastSelectedPage]->setSelected(false);
	}

	this->currentPage = page;

	size_t pdfPage = size_t_npos;

	if (page != size_t_npos && page < viewPagesLen)
	{
		PageView* vp = viewPages[page];
		vp->setSelected(true);
		lastSelectedPage = page;
		pdfPage = vp->getPage()->getPdfPageNr();
	}

	control->updatePageNumbers(currentPage, pdfPage);

	control->updateBackgroundSizeButton();
}

Control* XournalView::getControl()
{
	XOJ_CHECK_TYPE(XournalView);

	return control;
}

void XournalView::scrollToXY(size_t pageNo, double xDocument, double yDocument)
{
	XOJ_CHECK_TYPE(XournalView);
	double xOffset, yOffset;

	if (pageNo == size_t_npos || pageNo >= this->viewPagesLen)
	{
		return;
	}

	PageView* v = this->viewPages[pageNo];

	Layout* layout = gtk_xournal_get_layout(this->widget);

	double zoom = getZoom();


	xOffset = v->layout.getLayoutAbsoluteX(); //+ xDocument;
	yOffset = v->layout.getLayoutAbsoluteY() + yDocument;  //* zoom)/deltaZoom;

//    xOffset = xDocument; // + (zoom/deltaZoom);
//    yOffset = yDocument; // + (zoom/deltaZoom);
//    std::cout<<"xDocument: "<<xDocument<<" yDocument: "<<yDocument<<"\n";

	layout->ensureRectIsVisible(xDocument, yOffset, v->getDisplayWidth(), v->getDisplayHeight());

}
void XournalView::scrollTo(size_t pageNo, double yDocument)
{
	XOJ_CHECK_TYPE(XournalView);

	if (pageNo == size_t_npos || pageNo >= this->viewPagesLen)
	{
		return;
	}

	PageView* v = this->viewPages[pageNo];

	Layout* layout = gtk_xournal_get_layout(this->widget);
	layout->ensureRectIsVisible(v->layout.getLayoutAbsoluteX(), v->layout.getLayoutAbsoluteY() + yDocument,
								v->getDisplayWidth(), v->getDisplayHeight());
}

void XournalView::endTextAllPages(PageView* except)
{
	XOJ_CHECK_TYPE(XournalView);

	for (size_t i = 0; i < this->viewPagesLen; i++)
	{
		PageView* v = this->viewPages[i];
		if (except != v)
		{
			v->endText();
		}
	}
}

void XournalView::layerChanged(size_t page)
{
	XOJ_CHECK_TYPE(XournalView);

	if (page != size_t_npos && page < this->viewPagesLen)
	{
		this->viewPages[page]->rerenderPage();
	}
}

void XournalView::getPasteTarget(double& x, double& y)
{
    XOJ_CHECK_TYPE(XournalView);
    
    size_t pageNo = getCurrentPage();
    if (pageNo == size_t_npos)
    {
    	return;
    }
    
    Rectangle* rect = getVisibleRect(pageNo); //don't quite like it
    
    double pageX, pageY, pageW, pageH;
    int currentPage = this->getCurrentPage();
    XournalView::isPageVisibleXY(currentPage, &pageX, &pageY, &pageW, &pageH);


    if (rect)
    {
	Layout* layout = gtk_xournal_get_layout(this->widget);
	double layoutHeight = layout->layoutHeight;
	double visiblePageTop = layout->getVisiblePageTop(pageNo); //not sure if it's useful or not

	double pageOffset = 848; 
	int numPages = control->getDocument()->getPageCount();

	if(pageNo==0)
		pageOffset=0;
	else
		pageOffset=((layoutHeight/numPages)/getZoom());

	x = rect->width / 2;
	y = pageY - pageOffset*pageNo;
	//y = rect->height / 2;
	std::cout<<"pageNo:: "<<pageNo<<"\n";
	std::cout<<"pageY:: "<<pageY<<"\n";
	std::cout<<"visiblePageTop: "<<visiblePageTop<<"\n";;
	std::cout<<"layoutHeight: "<<layoutHeight<<"\n";;
	std::cout<<"pageOffset: "<<pageOffset<<"\n";;
	delete rect;
    }

}

/**
 * Return the rectangle which is visible on screen, in document cooordinates
 *
 * Or NULL if the page is not visible
 */
Rectangle* XournalView::getVisibleRect(size_t page)
{
	XOJ_CHECK_TYPE(XournalView);

	if (page == size_t_npos || page >= this->viewPagesLen)
	{
		return NULL;
	}
	PageView* p = this->viewPages[page];

	return gtk_xournal_get_visible_area(this->widget, p);
}

GtkWidget* XournalView::getWidget()
{
	XOJ_CHECK_TYPE(XournalView);

	return widget;
}

void XournalView::zoomIn()
{
	XOJ_CHECK_TYPE(XournalView);

	control->getZoomControl()->zoomIn();
}

void XournalView::zoomOut()
{
	XOJ_CHECK_TYPE(XournalView);

	control->getZoomControl()->zoomOut();
}

void XournalView::ensureRectIsVisible(int x, int y, int width, int heigth)
{
	XOJ_CHECK_TYPE(XournalView);

	Layout* layout = gtk_xournal_get_layout(this->widget);
	layout->ensureRectIsVisible(x, y, width, heigth);
}

double getSign(double x)
{
    return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}
void XournalView::zoomChanged(double lastZoom)
{
    XOJ_CHECK_TYPE(XournalView);
    
    Layout* layout = gtk_xournal_get_layout(this->widget);
    int currentPage = this->getCurrentPage();
    //double pageTop = layout->getVisiblePageTop(currentPage);
    double pageTop = 0.0;
    
    layout->layoutPages();
    
    double pageX, pageY;
    double pageW, pageH;
    XournalView::isPageVisibleXY(currentPage, &pageX, &pageY, &pageW, &pageH);
    //std::cout<<"currentZoom: "<<getZoom()<<"\nLast Zoom: "<<lastZoom<<"\n";
    //pageX = (pageX/2) + (pageX*(getZoom());
    //pageY = (pageY - 119 * (getZoom()/5) ) + getZoom()/(getZoom()-lastZoom);
    //std::cout<<"new pageX: "<<pageX<<" new pageY: "<<pageY<<"\n";
    
    double deltaZoom=(getZoom()-lastZoom);
    
    //std::cout<<"deltaZoom: "<<deltaZoom<<"\n";
    
    //this->scrollTo(currentPage, pageTop);	//Don't qute like it

    double wOffset = pageW * deltaZoom * 1.5;
    double hOffset = pageH * deltaZoom;

    //bias
    if (getSign(deltaZoom) < 0 ){
        hOffset-=8;
        wOffset+=8;
    }else{
        hOffset-=2;
        wOffset+=2;
    }

    pageY = pageY * getZoom() + hOffset;
    pageX = pageX * getZoom() + wOffset; //(pageW*(deltaZoom*1.5));

    //std::cout<<"pageY: "<<pageY<<"\n";
    //std::cout<<"pageH: "<<pageH<<"\n";

    //this->scrollToXY(currentPage, pageX, pageY);	//don't quite like it ATM
    
    Document* doc = control->getDocument();
    doc->lock();
    path file = doc->getEvMetadataFilename();
    doc->unlock();
    
    control->getMetadataManager()->setDouble(file, "zoom", getZoom());
    
    this->control->getScheduler()->blockRerenderZoom();
}

void XournalView::pageSizeChanged(size_t page)
{
	XOJ_CHECK_TYPE(XournalView);
	layoutPages();
}

void XournalView::pageChanged(size_t page)
{
	XOJ_CHECK_TYPE(XournalView);

	if (page != size_t_npos && page < this->viewPagesLen)
	{
		this->viewPages[page]->rerenderPage();
	}
}

void XournalView::pageDeleted(size_t page)
{
	XOJ_CHECK_TYPE(XournalView);

	size_t currentPage = control->getCurrentPageNo();

	delete this->viewPages[page];

	for (size_t i = page; i < this->viewPagesLen - 1; i++)
	{
		this->viewPages[i] = this->viewPages[i + 1];
	}

	this->viewPagesLen--;
	this->viewPages[this->viewPagesLen] = NULL;

	if (currentPage >= page)
	{
		currentPage--;
	}

	layoutPages();
	control->getScrollHandler()->scrollToPage(currentPage);
}

TextEditor* XournalView::getTextEditor()
{
	XOJ_CHECK_TYPE(XournalView);

	for (size_t i = 0; i < this->viewPagesLen; i++)
	{
		PageView* v = this->viewPages[i];
		if (v->getTextEditor())
		{
			return v->getTextEditor();
		}
	}

	return NULL;
}

void XournalView::resetShapeRecognizer()
{
	XOJ_CHECK_TYPE(XournalView);

	for (size_t i = 0; i < this->viewPagesLen; i++)
	{
		PageView* v = this->viewPages[i];
		v->resetShapeRecognizer();
	}
}

PdfCache* XournalView::getCache()
{
	XOJ_CHECK_TYPE(XournalView);

	return this->cache;
}

void XournalView::pageInserted(size_t page)
{
	XOJ_CHECK_TYPE(XournalView);

	PageView** lastViewPages = this->viewPages;

	this->viewPages = new PageView *[this->viewPagesLen + 1];

	for (size_t i = 0; i < page; i++)
	{
		this->viewPages[i] = lastViewPages[i];

		// unselect to prevent problems...
		this->viewPages[i]->setSelected(false);
	}

	for (size_t i = page; i < this->viewPagesLen; i++)
	{
		this->viewPages[i + 1] = lastViewPages[i];

		// unselect to prevent problems...
		this->viewPages[i + 1]->setSelected(false);
	}

	this->lastSelectedPage = -1;

	this->viewPagesLen++;

	delete[] lastViewPages;

	Document* doc = control->getDocument();
	doc->lock();
	PageView* pageView = new PageView(this, doc->getPage(page));
	doc->unlock();

	this->viewPages[page] = pageView;

	Layout* layout = gtk_xournal_get_layout(this->widget);
	layout->layoutPages();
	layout->checkSelectedPage();
}

double XournalView::getZoom()
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = getCurrentPage();
	if (p != size_t_npos && p < viewPagesLen)
	{
		PageView* page = viewPages[p];
		if (this->getControl()->getSettings()->isPresentationMode())
		{
			double heightZoom = this->getDisplayHeight() / page->getHeight();
			double widthZoom = this->getDisplayWidth() / page->getWidth();
			return (heightZoom < widthZoom) ? heightZoom : widthZoom;
		}
	}

	return control->getZoomControl()->getZoom();
}

void XournalView::updateXEvents()
{
	XOJ_CHECK_TYPE(XournalView);

	gtk_xournal_update_xevent(this->widget);
}

void XournalView::clearSelection()
{
	XOJ_CHECK_TYPE(XournalView);

	EditSelection* sel = GTK_XOURNAL(widget)->selection;
	GTK_XOURNAL(widget)->selection = NULL;
	delete sel;

	control->setClipboardHandlerSelection(getSelection());

	getCursor()->setMouseSelectionType(CURSOR_SELECTION_NONE);
	control->getToolHandler()->setSelectionEditTools(false, false);
}

void XournalView::deleteSelection(EditSelection* sel)
{
	XOJ_CHECK_TYPE(XournalView);

	if (sel == NULL)
	{
		sel = getSelection();
	}

	if (sel)
	{
		PageView* view = sel->getView();
		DeleteUndoAction* undo = new DeleteUndoAction(sel->getSourcePage(), false);
		sel->fillUndoItem(undo);
		control->getUndoRedoHandler()->addUndoAction(undo);

		clearSelection();

		view->rerenderPage();
		repaintSelection(true);
	}
}

void XournalView::setSelection(EditSelection* selection)
{
	XOJ_CHECK_TYPE(XournalView);

	clearSelection();
	GTK_XOURNAL(this->widget)->selection = selection;

	control->setClipboardHandlerSelection(getSelection());

	bool canChangeSize = false;
	bool canChangeColor = false;

	for (Element* e : *selection->getElements())
	{
		if (e->getType() == ELEMENT_TEXT)
		{
			canChangeColor = true;
		}
		else if (e->getType() == ELEMENT_STROKE)
		{
			Stroke* s = (Stroke*) e;
			if (s->getToolType() != STROKE_TOOL_ERASER)
			{
				canChangeColor = true;
			}
			canChangeSize = true;
		}

		if (canChangeColor && canChangeSize)
		{
			break;
		}
	}

	control->getToolHandler()->setSelectionEditTools(canChangeColor, canChangeSize);

	repaintSelection();
}

void XournalView::repaintSelection(bool evenWithoutSelection)
{
	XOJ_CHECK_TYPE(XournalView);

	if (evenWithoutSelection)
	{
		gtk_widget_queue_draw(this->widget);
		return;
	}

	EditSelection* selection = getSelection();
	if (selection == NULL)
	{
		return;
	}

	//	Redrawable * red = selection->getView();
	//	double zoom = getZoom();
	//	int x0 = red->getX();
	//	int y0 = red->getY();
	//	int x = selection->getX() * zoom;
	//	int y = selection->getY() * zoom;
	//	int w = selection->getWidth() * zoom;
	//	int h = selection->getHeight() * zoom;
	//
	//	gtk_xournal_repaint_area(this->widget, x0 + x - 10, y0 + y - 10, w + 20, h + 20);

	// TODO OPTIMIZE ?
	gtk_widget_queue_draw(this->widget);
}

void XournalView::layoutPages()
{
	XOJ_CHECK_TYPE(XournalView);

	Layout* layout = gtk_xournal_get_layout(this->widget);
	layout->layoutPages();
}

int XournalView::getDisplayHeight() const {
	XOJ_CHECK_TYPE(XournalView);

	GtkAllocation allocation = { 0 };
	gtk_widget_get_allocation(this->widget, &allocation);
	return allocation.height;
}

int XournalView::getDisplayWidth() const {
	XOJ_CHECK_TYPE(XournalView);

	GtkAllocation allocation = { 0 };
	gtk_widget_get_allocation(this->widget, &allocation);
	return allocation.width;
}

//DOUBLE conversion
bool XournalView::isPageVisibleXY(int page, double* x, double* y, double* visibleWidth, double* visibleHeight)
{
	XOJ_CHECK_TYPE(XournalView);

	Rectangle* rect = getVisibleRect(page);
	if (rect)
	{
		if (visibleHeight)
		{
			*visibleHeight = (double)rect->height;
			*y = (double)rect->y;
		}

		if (visibleWidth)
		{
			*visibleWidth = (double)rect->width;
			*x = (double)rect->x;
		}

		delete rect;
		return true;
	}
	if (visibleHeight)
	{
		*visibleHeight = 0;
	}

	return false;
}

bool XournalView::isPageVisible(int page, int* visibleHeight)  
{
	XOJ_CHECK_TYPE(XournalView);

	Rectangle* rect = getVisibleRect(page);
	if (rect)
	{
		if (visibleHeight)
		{
			*visibleHeight = rect->height;
		}

		delete rect;
		return true;
	}
	if (visibleHeight)
	{
		*visibleHeight = 0;
	}

	return false;
}

void XournalView::documentChanged(DocumentChangeType type)
{
	XOJ_CHECK_TYPE(XournalView);

	if (type != DOCUMENT_CHANGE_CLEARED && type != DOCUMENT_CHANGE_COMPLETE)
	{
		return;
	}

	XournalScheduler* scheduler = this->control->getScheduler();
	scheduler->lock();
	scheduler->removeAllJobs();

	clearSelection();

	for (size_t i = 0; i < this->viewPagesLen; i++)
	{
		delete this->viewPages[i];
	}
	delete[] this->viewPages;

	Document* doc = control->getDocument();
	doc->lock();

	this->viewPagesLen = doc->getPageCount();
	this->viewPages = new PageView*[viewPagesLen];

	for (size_t i = 0; i < viewPagesLen; i++)
	{
		PageView* pageView = new PageView(this, doc->getPage(i));
		viewPages[i] = pageView;
	}

	doc->unlock();

	layoutPages();
	scrollTo(0, 0);

	scheduler->unlock();
}

bool XournalView::cut()
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = getCurrentPage();
	if (p == size_t_npos || p >= viewPagesLen)
	{
		return false;
	}

	PageView* page = viewPages[p];
	return page->cut();
}

bool XournalView::copy()
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = getCurrentPage();
	if (p == size_t_npos || p >= viewPagesLen)
	{
		return false;
	}

	PageView* page = viewPages[p];
	return page->copy();
}

bool XournalView::paste()
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = getCurrentPage();
	if (p == size_t_npos || p >= viewPagesLen)
	{
		return false;
	}

	PageView* page = viewPages[p];
	return page->paste();
}

bool XournalView::actionDelete()
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = getCurrentPage();
	if (p == size_t_npos || p >= viewPagesLen)
	{
		return false;
	}

	PageView* page = viewPages[p];
	return page->actionDelete();
}

Document* XournalView::getDocument()
{
	XOJ_CHECK_TYPE(XournalView);

	return control->getDocument();
}

ArrayIterator<PageView*> XournalView::pageViewIterator()
{
	XOJ_CHECK_TYPE(XournalView);

	return ArrayIterator<PageView*> (viewPages, viewPagesLen);
}

PagePositionHandler* XournalView::getPagePositionHandler()
{
	XOJ_CHECK_TYPE(XournalView);

	return this->pagePosition;
}

Cursor* XournalView::getCursor()
{
	XOJ_CHECK_TYPE(XournalView);

	return control->getCursor();
}

EditSelection* XournalView::getSelection()
{
	XOJ_CHECK_TYPE(XournalView);

	g_return_val_if_fail(this->widget != NULL, NULL);
	g_return_val_if_fail(GTK_IS_XOURNAL(this->widget), NULL);

	return GTK_XOURNAL(this->widget)->selection;
}
