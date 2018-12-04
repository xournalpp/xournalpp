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

#include <gdk/gdk.h>

#include <math.h>

XournalView::XournalView(GtkWidget* parent, Control* control)
{
	XOJ_INIT_TYPE(XournalView);

	this->parent = GTK_CONTAINER(parent);

	this->control = control;
	this->cache = new PdfCache(control->getSettings()->getPdfPageCacheSize());
	registerListener(control);

	this->widget = gtk_xournal_new(this, GTK_SCROLLABLE(parent));
	// we need to refer widget here, because we unref it somewhere twice!?
	g_object_ref(this->widget);

	gtk_container_add(GTK_CONTAINER(parent), this->widget);
	gtk_widget_show(this->widget);

	g_signal_connect(getWidget(), "realize", G_CALLBACK(onRealized), this);

	this->repaintHandler = new RepaintHandler(this);
	this->pagePosition = new PagePositionHandler();

	this->viewPages = NULL;
	this->viewPagesLen = 0;
	this->margin = 75;
	this->currentPage = 0;
	this->lastSelectedPage = -1;
	this->lastPenAction = 0;

	control->getZoomControl()->addZoomListener(this);

	gtk_widget_set_can_default(this->widget, true);
	gtk_widget_grab_default(this->widget);

	gtk_widget_grab_focus(this->widget);

	this->cleanupTimeout = g_timeout_add_seconds(5, (GSourceFunc) clearMemoryTimer, this);
	// pinch-to-zoom
	this->zoom_gesture_active = false;

	// use parent as the gestures widget and not this->widget as gesture gets
	// buggy otherwise (scrolling interferes with gestures scale value)
	this->zoom_gesture = gtk_gesture_zoom_new(parent);

	gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(this->zoom_gesture), GTK_PHASE_CAPTURE);
	g_signal_connect(this->zoom_gesture, "begin", G_CALLBACK (zoom_gesture_begin_cb), this);
	g_signal_connect(this->zoom_gesture, "scale-changed", G_CALLBACK (zoom_gesture_scale_changed_cb), this);
	g_signal_connect(this->zoom_gesture, "end", G_CALLBACK (zoom_gesture_end_cb), this);
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

	gtk_widget_destroy(this->widget);
	this->widget = NULL;

	XOJ_RELEASE_TYPE(XournalView);
}

gint pageViewCmpSize(XojPageView* a, XojPageView* b)
{
	return a->getLastVisibleTime() - b->getLastVisibleTime();
}

void XournalView::staticLayoutPages(GtkWidget *widget, GtkAllocation *allocation, void *data)
{
	XournalView *xv = (XournalView *)data;
	XOJ_CHECK_TYPE_OBJ(xv, XournalView);
	xv->layoutPages();
}

gboolean XournalView::clearMemoryTimer(XournalView* widget)
{
	XOJ_CHECK_TYPE_OBJ(widget, XournalView);

	GList* list = NULL;

	for (size_t i = 0; i < widget->viewPagesLen; i++)
	{
		XojPageView* v = widget->viewPages[i];
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
			XojPageView* v = (XojPageView*) l->data;

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
		XojPageView* v = this->viewPages[p];
		if (v->onKeyPressEvent(event))
		{
			return true;
		}
	}

	// Esc leaves fullscreen mode
	if (event->keyval == GDK_KEY_Escape || event->keyval == GDK_KEY_F11)
	{
		if (control->isFullscreen())
		{
			control->enableFullscreen(false);
			return true;
		}
	}

	// F5 starts presentation modus
	if (event->keyval == GDK_KEY_F5)
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
		GtkAllocation alloc =
		{ 0 };
		gtk_widget_get_allocation(gtk_widget_get_parent(this->widget), &alloc);
		int windowHeight = alloc.height - scrollKeySize;

		if (event->keyval == GDK_KEY_Page_Down)
		{
			layout->scrollRelativ(0, windowHeight);
			return false;
		}
		if (event->keyval == GDK_KEY_Page_Up || event->keyval == GDK_KEY_space)
		{
			layout->scrollRelativ(0, -windowHeight);
			return true;
		}
	}
	else
	{
		if (event->keyval == GDK_KEY_Page_Down)
		{
			control->getScrollHandler()->goToNextPage();
			return true;
		}
		if (event->keyval == GDK_KEY_Page_Up)
		{
			control->getScrollHandler()->goToPreviousPage();
			return true;
		}
	}

	if (event->keyval == GDK_KEY_space) {
		GtkAllocation alloc = { 0 };
		gtk_widget_get_allocation(gtk_widget_get_parent(this->widget), &alloc);
		int windowHeight = alloc.height - scrollKeySize;

		layout->scrollRelativ(0, windowHeight);
		return true;
	}

	if (event->keyval == GDK_KEY_Up)
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

	if (event->keyval == GDK_KEY_Down)
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

	if (event->keyval == GDK_KEY_Left)
	{
		//if (control->getSettings()->isPresentationMode())
		//{
			control->getScrollHandler()->goToPreviousPage();
			return true;
		//}
		//layout->scrollRelativ(-scrollKeySize, 0);
		//return true;
	}

	if (event->keyval == GDK_KEY_Right)
	{
		//if (control->getSettings()->isPresentationMode())
		//{
			control->getScrollHandler()->goToNextPage();
			return true;
		//}
		//layout->scrollRelativ(scrollKeySize, 0);
		//return true;
	}

	if (event->keyval == GDK_KEY_End)
	{
		control->getScrollHandler()->goToLastPage();
		return true;
	}

	if (event->keyval == GDK_KEY_Home)
	{
		control->getScrollHandler()->goToFirstPage();
		return true;
	}

	// vim like scrolling
	if (event->keyval == GDK_KEY_j)
	{
		layout->scrollRelativ(0, 60);
		return true;
	}
	if (event->keyval == GDK_KEY_k)
	{
		layout->scrollRelativ(0, -60);
		return true;
	}
	if (event->keyval == GDK_KEY_h)
	{
		layout->scrollRelativ(-60, 0);
		return true;
	}
	if (event->keyval == GDK_KEY_l)
	{
		layout->scrollRelativ(60, 0);
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
		XojPageView* v = this->viewPages[p];
		if (v->onKeyReleaseEvent(event))
		{
			return true;
		}
	}

	return false;
}

void XournalView::onRealized(GtkWidget* widget, XournalView* view)
{
	XOJ_CHECK_TYPE_OBJ(view, XournalView);

	view->setEventCompression(view->getControl()->getSettings()->isEventCompression());
}

void XournalView::zoom_gesture_begin_cb(GtkGesture* gesture, GdkEventSequence* sequence, XournalView* view)
{
	XOJ_CHECK_TYPE_OBJ(view, XournalView);

	if (view->shouldIgnoreTouchEvents())
	{
		return;
	}

	Layout* layout = gtk_xournal_get_layout(view->widget);
	// Save visible rectangle at beginning of gesture
	view->visRect_gesture_begin = layout->getVisibleRect();

	view->zoom_gesture_begin = view->getZoom();
	view->zoom_gesture_active = true;

	// get center of bounding box
	ZoomControl* zoom = view->control->getZoomControl();
	gtk_gesture_get_bounding_box_center(GTK_GESTURE(gesture), &zoom->zoom_center_x, &zoom->zoom_center_y);
}

void XournalView::zoom_gesture_end_cb(GtkGesture* gesture, GdkEventSequence* sequence, XournalView* view)
{
	XOJ_CHECK_TYPE_OBJ(view, XournalView);

	ZoomControl* zoom = view->control->getZoomControl();
	zoom->zoom_center_x = -1;
	zoom->zoom_center_y = -1;
	view->zoom_gesture_active = false;
}

void XournalView::zoom_gesture_scale_changed_cb(GtkGestureZoom* gesture, gdouble scale, XournalView* view)
{
	XOJ_CHECK_TYPE_OBJ(view, XournalView);

	if (view->shouldIgnoreTouchEvents())
	{
		return;
	}

	view->setZoom(scale * view->zoom_gesture_begin);
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
	XojPageView* v = this->viewPages[p];

	return v->searchTextOnPage(text, occures, top);
}

void XournalView::forceUpdatePagenumbers()
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = this->currentPage;
	this->currentPage = size_t_npos;

	control->firePageSelected(p);
}

XojPageView* XournalView::getViewFor(size_t pageNr)
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

	control->getMetadataManager()->storeMetadata(file.c_str(), page, getZoom());

	if (this->lastSelectedPage != size_t_npos && this->lastSelectedPage < this->viewPagesLen)
	{
		this->viewPages[this->lastSelectedPage]->setSelected(false);
	}

	this->currentPage = page;

	size_t pdfPage = size_t_npos;

	if (page != size_t_npos && page < viewPagesLen)
	{
		XojPageView* vp = viewPages[page];
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

void XournalView::scrollTo(size_t pageNo, double yDocument)
{
	XOJ_CHECK_TYPE(XournalView);

	if (pageNo >= this->viewPagesLen)
	{
		return;
	}

	XojPageView* v = this->viewPages[pageNo];

	Layout* layout = gtk_xournal_get_layout(this->widget);
	layout->ensureRectIsVisible(v->layout.getLayoutAbsoluteX(), v->layout.getLayoutAbsoluteY() + yDocument,
								v->getDisplayWidth(), v->getDisplayHeight());
}

void XournalView::endTextAllPages(XojPageView* except)
{
	XOJ_CHECK_TYPE(XournalView);

	for (size_t i = 0; i < this->viewPagesLen; i++)
	{
		XojPageView* v = this->viewPages[i];
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

	Rectangle* rect = getVisibleRect(pageNo);

	if (rect)
	{
		x = rect->width / 2;
		y = rect->height / 2;
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
	XojPageView* p = this->viewPages[page];

	return getVisibleRect(p);
}

Rectangle* XournalView::getVisibleRect(XojPageView* redrawable)
{
	XOJ_CHECK_TYPE(XournalView);

	return gtk_xournal_get_visible_area(this->widget, redrawable);
}

GtkContainer* XournalView::getParent()
{
	XOJ_CHECK_TYPE(XournalView);

	return this->parent;
}

/**
 * A pen action was detected now, therefore ignore touch events
 * for a short time
 */
void XournalView::penActionDetected()
{
	XOJ_CHECK_TYPE(XournalView);

	this->lastPenAction = g_get_monotonic_time() / 1000;
}

/**
 * If the pen was active a short time before, ignore touch events
 */
bool XournalView::shouldIgnoreTouchEvents()
{
	XOJ_CHECK_TYPE(XournalView);

	if ((g_get_monotonic_time() / 1000 - this->lastPenAction) < 1000)
	{
		// printf("Ignore touch, pen was active\n");
		return true;
	}

	return false;
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

void XournalView::setZoom(gdouble scale)
{
	XOJ_CHECK_TYPE(XournalView);
	control->getZoomControl()->setZoom(scale);
}

void XournalView::ensureRectIsVisible(int x, int y, int width, int height)
{
	XOJ_CHECK_TYPE(XournalView);

	Layout* layout = gtk_xournal_get_layout(this->widget);
	layout->ensureRectIsVisible(x, y, width, height);
}

void XournalView::zoomChanged(double lastZoom)
{
	XOJ_CHECK_TYPE(XournalView);

	Layout* layout = gtk_xournal_get_layout(this->widget);
	int currentPage = this->getCurrentPage();
	XojPageView* view = getViewFor(currentPage);
	ZoomControl* zoom = control->getZoomControl();

	if (!view)
	{
		return;
	}

	// move this somewhere else maybe
	layout->layoutPages();

	// Keep zoom center at static position in current view
	// by scrolling relative to counter motion induced by zoom
	// in orignal version top left corner of first page static
	// Pack into extra function later
	double zoom_now = getZoom();
	// relative scrolling
	double zoom_eff = zoom_now / lastZoom;
	int scroll_x;
	int scroll_y;
	// x,y position of visible rectangle for gesture scrolling
	int vis_x;
	int vis_y;
	// get margins for relative scroll calculation
	double marginLeft = (double) view->layout.getMarginLeft();
	double marginTop = (double) view->layout.getMarginTop();

	// Absolute centred scrolling used for gesture
	if (this->zoom_gesture_active)
	{
		vis_x = (int) ((zoom->zoom_center_x - marginLeft) * (zoom_now / this->zoom_gesture_begin - 1));
		vis_y = (int) ((zoom->zoom_center_y - marginTop) * (zoom_now / this->zoom_gesture_begin - 1));
		layout->scrollAbs(this->visRect_gesture_begin.x + vis_x, this->visRect_gesture_begin.y + vis_y);
	}

	// Relative centered scrolling used for SHIFT-mousewheel
	if (zoom_eff != 1 && zoom->zoom_center_x != -1 && this->zoom_gesture_active == false)
	{
		scroll_x = (int) ((zoom->zoom_center_x - marginLeft) * (zoom_eff - 1));
		scroll_y = (int) ((zoom->zoom_center_y - marginTop) * (zoom_eff - 1));

		// adjust view by scrolling
		layout->scrollRelativ(scroll_x, scroll_y);
	}

	Document* doc = control->getDocument();
	doc->lock();
	path file = doc->getEvMetadataFilename();
	doc->unlock();

	control->getMetadataManager()->storeMetadata(file.c_str(), getCurrentPage(), getZoom());

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
		XojPageView* v = this->viewPages[i];
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
		XojPageView* v = this->viewPages[i];
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

	XojPageView** lastViewPages = this->viewPages;

	this->viewPages = new XojPageView *[this->viewPagesLen + 1];

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
	XojPageView* pageView = new XojPageView(this, doc->getPage(page));
	doc->unlock();

	this->viewPages[page] = pageView;

	Layout* layout = gtk_xournal_get_layout(this->widget);
	layout->layoutPages();
	layout->updateCurrentPage();
}

double XournalView::getZoom()
{
	XOJ_CHECK_TYPE(XournalView);

	size_t p = getCurrentPage();
	if (p != size_t_npos && p < viewPagesLen)
	{
		XojPageView* page = viewPages[p];
		if (this->getControl()->getSettings()->isPresentationMode())
		{
			double heightZoom = this->getDisplayHeight() / page->getHeight();
			double widthZoom = this->getDisplayWidth() / page->getWidth();
			return (heightZoom < widthZoom) ? heightZoom : widthZoom;
		}
	}

	return control->getZoomControl()->getZoom();
}

int XournalView::getDpiScaleFactor()
{
	XOJ_CHECK_TYPE(XournalView);
	return gtk_widget_get_scale_factor(widget);
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
		XojPageView* view = sel->getView();
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

	// repaint always the whole widget
	gtk_widget_queue_draw(this->widget);
}

void XournalView::setEventCompression(gboolean enable)
{
	// Enable this when gdk is new enough for the compression feature.
	if (gtk_widget_get_realized(getWidget()))
	{
		gdk_window_set_event_compression(gtk_widget_get_window(getWidget()), FALSE);
	}
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

bool XournalView::isPageVisible(size_t page, int* visibleHeight)
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
	this->viewPages = new XojPageView*[viewPagesLen];

	for (size_t i = 0; i < viewPagesLen; i++)
	{
		XojPageView* pageView = new XojPageView(this, doc->getPage(i));
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

	XojPageView* page = viewPages[p];
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

	XojPageView* page = viewPages[p];
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

	XojPageView* page = viewPages[p];
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

	XojPageView* page = viewPages[p];
	return page->actionDelete();
}

Document* XournalView::getDocument()
{
	XOJ_CHECK_TYPE(XournalView);

	return control->getDocument();
}

ArrayIterator<XojPageView*> XournalView::pageViewIterator()
{
	XOJ_CHECK_TYPE(XournalView);

	return ArrayIterator<XojPageView*> (viewPages, viewPagesLen);
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
