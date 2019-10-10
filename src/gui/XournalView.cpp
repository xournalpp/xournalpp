#include "XournalView.h"

#include "Layout.h"
#include "PageView.h"
#include "RepaintHandler.h"
#include "Shadow.h"
#include "XournalppCursor.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "control/settings/MetadataManager.h"
#include "gui/inputdevices/HandRecognition.h"
#include "model/Document.h"
#include "model/Stroke.h"
#include "undo/DeleteUndoAction.h"
#include "widgets/XournalWidget.h"

#include "Rectangle.h"
#include "Util.h"
#include "util/cpp14memory.h"

#include <gdk/gdk.h>

#include <cmath>
#include <tuple>

XournalView::XournalView(GtkWidget* parent, Control* control, ScrollHandling* scrollHandling)
 : scrollHandling(scrollHandling)
 , control(control)
{
	this->cache = new PdfCache(control->getSettings()->getPdfPageCacheSize());
	registerListener(control);

	InputContext* inputContext = nullptr;
	if (this->control->getSettings()->getExperimentalInputSystemEnabled())
	{
		inputContext = new InputContext(this, scrollHandling);
		this->widget = gtk_xournal_new(this, inputContext);
	}
	else
	{
		this->widget = gtk_xournal_new_deprecated(this, scrollHandling);
	}
	// we need to refer widget here, because we unref it somewhere twice!?
	g_object_ref(this->widget);

	gtk_container_add(GTK_CONTAINER(parent), this->widget);
	gtk_widget_show(this->widget);

	g_signal_connect(getWidget(), "realize", G_CALLBACK(onRealized), this);

	this->repaintHandler = new RepaintHandler(this);
	this->handRecognition = new HandRecognition(this->widget, inputContext, control->getSettings());

	control->getZoomControl()->addZoomListener(this);

	gtk_widget_set_can_default(this->widget, true);
	gtk_widget_grab_default(this->widget);

	gtk_widget_grab_focus(this->widget);

	this->cleanupTimeout = g_timeout_add_seconds(5, (GSourceFunc) clearMemoryTimer, this);
}

XournalView::~XournalView()
{
	g_source_remove(this->cleanupTimeout);

	for (size_t i = 0; i < this->viewPagesLen; i++)
	{
		delete this->viewPages[i];
	}
	delete[] this->viewPages;
	this->viewPagesLen = 0;
	this->viewPages = nullptr;

	delete this->cache;
	this->cache = nullptr;
	delete this->repaintHandler;
	this->repaintHandler = nullptr;


	gtk_widget_destroy(this->widget);
	this->widget = nullptr;

	delete this->handRecognition;
	this->handRecognition = nullptr;
}

gint pageViewIncreasingClockTime(XojPageView* a, XojPageView* b)
{
	return a->getLastVisibleTime() - b->getLastVisibleTime();  // >0 will put a after b
}

void XournalView::staticLayoutPages(GtkWidget* widget, GtkAllocation* allocation, void* data)
{
	XournalView* xv = (XournalView*) data;
	xv->layoutPages();
}

gboolean XournalView::clearMemoryTimer(XournalView* widget)
{
	GList* list = nullptr;

	for (size_t i = 0; i < widget->viewPagesLen; i++)
	{
		XojPageView* v = widget->viewPages[i];
		if (v->getLastVisibleTime() > 0)
		{
			list = g_list_insert_sorted(list, v, (GCompareFunc) pageViewIncreasingClockTime);
		}
	}

	int pixel = 2884560;
	int firstPages = 4;

	int i = 0;

	for (GList* l = g_list_last(list); l != nullptr; l = l->prev)  // older (higher time) to newer (lower time)
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
	return currentPage;
}

const int scrollKeySize = 30;

bool XournalView::onKeyPressEvent(GdkEventKey* event)
{
	size_t p = getCurrentPage();
	if (p != npos && p < this->viewPagesLen)
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
			control->setFullscreen(false);
			return true;
		}
	}

	// F5 starts presentation modus
	if (event->keyval == GDK_KEY_F5)
	{
		if (!control->isFullscreen())
		{
			control->setViewPresentationMode(true);
			control->setFullscreen(true);
			return true;
		}
	}

	guint state = event->state & gtk_accelerator_get_default_mod_mask();

	Layout* layout = gtk_xournal_get_layout(this->widget);

	if (state & GDK_SHIFT_MASK)
	{
		GtkAllocation alloc = {0};
		gtk_widget_get_allocation(gtk_widget_get_parent(this->widget), &alloc);
		int windowHeight = alloc.height - scrollKeySize;

		if (event->keyval == GDK_KEY_Page_Down)
		{
			layout->scrollRelative(0, windowHeight);
			return false;
		}
		if (event->keyval == GDK_KEY_Page_Up || event->keyval == GDK_KEY_space)
		{
			layout->scrollRelative(0, -windowHeight);
			return true;
		}
	}
	else
	{
		if (event->keyval == GDK_KEY_Page_Down || event->keyval == GDK_KEY_KP_Page_Down)
		{
			control->getScrollHandler()->goToNextPage();
			return true;
		}
		if (event->keyval == GDK_KEY_Page_Up || event->keyval == GDK_KEY_KP_Page_Up)
		{
			control->getScrollHandler()->goToPreviousPage();
			return true;
		}
	}

	if (event->keyval == GDK_KEY_space) {
		GtkAllocation alloc = {0};
		gtk_widget_get_allocation(gtk_widget_get_parent(this->widget), &alloc);
		int windowHeight = alloc.height - scrollKeySize;

		layout->scrollRelative(0, windowHeight);
		return true;
	}

	// Numeric keypad always navigates by page
	if (event->keyval == GDK_KEY_KP_Up)
	{
		this->pageRelativeXY(0, -1);
		return true;
	}

	if (event->keyval == GDK_KEY_KP_Down)
	{
		this->pageRelativeXY(0, 1);
		return true;
	}

	if (event->keyval == GDK_KEY_KP_Left)
	{
		this->pageRelativeXY(-1, 0);
		return true;
	}

	if (event->keyval == GDK_KEY_KP_Right)
	{
		this->pageRelativeXY(1, 0);
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
			if (state & GDK_SHIFT_MASK)
			{
				this->pageRelativeXY(0, -1);
			}
			else
			{
				layout->scrollRelative(0, -scrollKeySize);
			}
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
			if (state & GDK_SHIFT_MASK)
			{
				this->pageRelativeXY(0, 1);
			}
			else
			{
				layout->scrollRelative(0, scrollKeySize);
			}
			return true;
		}
	}

	if (event->keyval == GDK_KEY_Left)
	{
		if (state & GDK_SHIFT_MASK)
		{
			this->pageRelativeXY(-1, 0);
		}
		else
		{
			if (control->getSettings()->isPresentationMode())
			{
				control->getScrollHandler()->goToPreviousPage();
			}
			else
			{
				layout->scrollRelative(-scrollKeySize, 0);
			}
		}
		return true;
	}

	if (event->keyval == GDK_KEY_Right)
	{
		if (state & GDK_SHIFT_MASK)
		{
			this->pageRelativeXY(1, 0);
		}
		else
		{
			if (control->getSettings()->isPresentationMode())
			{
				control->getScrollHandler()->goToNextPage();
			}
			else
			{
				layout->scrollRelative(scrollKeySize, 0);
			}
		}
		return true;
	}

	if (event->keyval == GDK_KEY_End || event->keyval == GDK_KEY_KP_End)
	{
		control->getScrollHandler()->goToLastPage();
		return true;
	}

	if (event->keyval == GDK_KEY_Home || event->keyval == GDK_KEY_KP_Home)
	{
		control->getScrollHandler()->goToFirstPage();
		return true;
	}

	// vim like scrolling
	if (event->keyval == GDK_KEY_j)
	{
		layout->scrollRelative(0, 60);
		return true;
	}
	if (event->keyval == GDK_KEY_k)
	{
		layout->scrollRelative(0, -60);
		return true;
	}
	if (event->keyval == GDK_KEY_h)
	{
		layout->scrollRelative(-60, 0);
		return true;
	}
	if (event->keyval == GDK_KEY_l)
	{
		layout->scrollRelative(60, 0);
		return true;
	}

	return false;
}

RepaintHandler* XournalView::getRepaintHandler()
{
	return this->repaintHandler;
}

bool XournalView::onKeyReleaseEvent(GdkEventKey* event)
{
	size_t p = getCurrentPage();
	if (p != npos && p < this->viewPagesLen)
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
	// Disable event compression
	if (gtk_widget_get_realized(view->getWidget()))
	{
		gdk_window_set_event_compression(gtk_widget_get_window(view->getWidget()), false);
	}
	else
	{
		g_warning("could not disable event compression");
	}
}

// send the focus back to the appropriate widget
void XournalView::requestFocus()
{
	gtk_widget_grab_focus(this->widget);
}

bool XournalView::searchTextOnPage(string text, size_t p, int* occures, double* top)
{
	if (p == npos || p >= this->viewPagesLen)
	{
		return false;
	}
	XojPageView* v = this->viewPages[p];

	return v->searchTextOnPage(text, occures, top);
}

void XournalView::forceUpdatePagenumbers()
{
	size_t p = this->currentPage;
	this->currentPage = npos;

	control->firePageSelected(p);
}

XojPageView* XournalView::getViewFor(size_t pageNr)
{
	if (pageNr == npos || pageNr >= this->viewPagesLen)
	{
		return nullptr;
	}
	return this->viewPages[pageNr];
}

void XournalView::pageSelected(size_t page)
{
	if (this->currentPage == page && this->lastSelectedPage == page)
	{
		return;
	}

	Document* doc = control->getDocument();
	doc->lock();
	Path file = doc->getEvMetadataFilename();
	doc->unlock();

	control->getMetadataManager()->storeMetadata(file.str(), page, getZoom());

	if (this->lastSelectedPage != npos && this->lastSelectedPage < this->viewPagesLen)
	{
		this->viewPages[this->lastSelectedPage]->setSelected(false);
	}

	this->currentPage = page;

	size_t pdfPage = npos;

	if (page != npos && page < viewPagesLen)
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
	return control;
}

void XournalView::scrollTo(size_t pageNo, double yDocument)
{
	if (pageNo >= this->viewPagesLen)
	{
		return;
	}

	XojPageView* v = this->viewPages[pageNo];

	// Make sure it is visible
	Layout* layout = gtk_xournal_get_layout(this->widget);

	int x = v->getX();
	int y = v->getY() + std::lround(yDocument);
	int width = v->getDisplayWidth();
	int height = v->getDisplayHeight();

	layout->ensureRectIsVisible(x, y, width, height);

	// Select the page
	control->firePageSelected(pageNo);
}


void XournalView::pageRelativeXY(int offCol, int offRow)
{
	size_t currPage = getCurrentPage();

	XojPageView* view = getViewFor(currPage);
	int row = view->getMappedRow();
	int col = view->getMappedCol();

	Layout* layout = gtk_xournal_get_layout(this->widget);
	auto optionalPageIndex = layout->getIndexAtGridMap(row + offRow, col + offCol);
	if (optionalPageIndex)
	{
		this->scrollTo(*optionalPageIndex, 0);
	}
}


void XournalView::endTextAllPages(XojPageView* except)
{
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
	if (page != npos && page < this->viewPagesLen)
	{
		this->viewPages[page]->rerenderPage();
	}
}

void XournalView::getPasteTarget(double& x, double& y)
{
	size_t pageNo = getCurrentPage();
	if (pageNo == npos)
	{
		return;
	}

	Rectangle* rect = getVisibleRect(pageNo);

	if (rect)
	{
		x = rect->x + rect->width / 2;
		y = rect->y + rect->height / 2;
		delete rect;
	}
}

/**
 * Return the rectangle which is visible on screen, in document cooordinates
 *
 * Or nullptr if the page is not visible
 */
Rectangle* XournalView::getVisibleRect(size_t page)
{
	if (page == npos || page >= this->viewPagesLen)
	{
		return nullptr;
	}
	XojPageView* p = this->viewPages[page];

	return getVisibleRect(p);
}

Rectangle* XournalView::getVisibleRect(XojPageView* redrawable)
{
	return gtk_xournal_get_visible_area(this->widget, redrawable);
}

/**
 * @return Helper class for Touch specific fixes
 */
HandRecognition* XournalView::getHandRecognition()
{
	return handRecognition;
}

/**
 * @returnScrollbars
 */
ScrollHandling* XournalView::getScrollHandling()
{
	return scrollHandling;
}

GtkWidget* XournalView::getWidget()
{
	return widget;
}

void XournalView::zoomIn()
{
	control->getZoomControl()->zoomOneStep(ZOOM_IN);
}

void XournalView::zoomOut()
{
	control->getZoomControl()->zoomOneStep(ZOOM_OUT);
}

void XournalView::ensureRectIsVisible(int x, int y, int width, int height)
{
	Layout* layout = gtk_xournal_get_layout(this->widget);
	layout->ensureRectIsVisible(x, y, width, height);
}

void XournalView::zoomChanged()
{
	Layout* layout = gtk_xournal_get_layout(this->widget);
	size_t currentPage = this->getCurrentPage();
	XojPageView* view = getViewFor(currentPage);
	ZoomControl* zoom = control->getZoomControl();

	if (!view)
	{
		return;
	}


	if (zoom->isZoomPresentationMode() || zoom->isZoomFitMode())
	{
		scrollTo(currentPage);
	}
	else
	{
		std::tuple<double, double> pos = zoom->getScrollPositionAfterZoom();
		if (std::get<0>(pos) != -1 && std::get<1>(pos) != -1)
		{
			layout->scrollAbs(std::get<0>(pos), std::get<1>(pos));
		}
	}
	// move this somewhere else maybe
	layout->recalculate();

	Document* doc = control->getDocument();
	doc->lock();
	Path file = doc->getEvMetadataFilename();
	doc->unlock();

	control->getMetadataManager()->storeMetadata(file.str(), getCurrentPage(), zoom->getZoomReal());

	// Updates the Eraser's cursor icon in order to make it as big as the erasing area
	control->getCursor()->updateCursor();

	this->control->getScheduler()->blockRerenderZoom();
}

void XournalView::pageSizeChanged(size_t page)
{
	layoutPages();
}

void XournalView::pageChanged(size_t page)
{
	if (page != npos && page < this->viewPagesLen)
	{
		this->viewPages[page]->rerenderPage();
	}
}

void XournalView::pageDeleted(size_t page)
{
	size_t currentPage = control->getCurrentPageNo();

	delete this->viewPages[page];

	for (size_t i = page; i < this->viewPagesLen - 1; i++)
	{
		this->viewPages[i] = this->viewPages[i + 1];
	}

	this->viewPagesLen--;
	this->viewPages[this->viewPagesLen] = nullptr;

	if (currentPage >= page)
	{
		currentPage--;
	}

	layoutPages();
	control->getScrollHandler()->scrollToPage(currentPage);
}

TextEditor* XournalView::getTextEditor()
{
	for (size_t i = 0; i < this->viewPagesLen; i++)
	{
		XojPageView* v = this->viewPages[i];
		if (v->getTextEditor())
		{
			return v->getTextEditor();
		}
	}

	return nullptr;
}

void XournalView::resetShapeRecognizer()
{
	for (size_t i = 0; i < this->viewPagesLen; i++)
	{
		XojPageView* v = this->viewPages[i];
		v->resetShapeRecognizer();
	}
}

PdfCache* XournalView::getCache()
{
	return this->cache;
}

void XournalView::pageInserted(size_t page)
{
	XojPageView** lastViewPages = this->viewPages;

	this->viewPages = new XojPageView*[this->viewPagesLen + 1];

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
	layout->recalculate();
	layout->updateVisibility();
}

double XournalView::getZoom()
{
	return control->getZoomControl()->getZoom();
}

int XournalView::getDpiScaleFactor()
{
	return gtk_widget_get_scale_factor(widget);
}

void XournalView::clearSelection()
{
	EditSelection* sel = GTK_XOURNAL(widget)->selection;
	GTK_XOURNAL(widget)->selection = nullptr;
	delete sel;

	control->setClipboardHandlerSelection(getSelection());

	getCursor()->setMouseSelectionType(CURSOR_SELECTION_NONE);
	control->getToolHandler()->setSelectionEditTools(false, false, false);
}

void XournalView::deleteSelection(EditSelection* sel)
{
	if (sel == nullptr)
	{
		sel = getSelection();
	}

	if (sel)
	{
		XojPageView* view = sel->getView();
		auto undo = mem::make_unique<DeleteUndoAction>(sel->getSourcePage(), false);
		sel->fillUndoItem(undo.get());
		control->getUndoRedoHandler()->addUndoAction(std::move(undo));

		clearSelection();

		view->rerenderPage();
		repaintSelection(true);
	}
}

void XournalView::setSelection(EditSelection* selection)
{
	clearSelection();
	GTK_XOURNAL(this->widget)->selection = selection;

	control->setClipboardHandlerSelection(getSelection());

	bool canChangeSize = false;
	bool canChangeColor = false;
	bool canChangeFill = false;

	for (Element* e: *selection->getElements())
	{
		if (e->getType() == ELEMENT_TEXT)
		{
			canChangeColor = true;
		}
		else if (e->getType() == ELEMENT_STROKE)
		{
			auto* s = static_cast<Stroke*>(e);
			if (s->getToolType() != STROKE_TOOL_ERASER)
			{
				canChangeColor = true;
				canChangeFill = true;
			}
			canChangeSize = true;
		}

		if (canChangeColor && canChangeSize && canChangeFill)
		{
			break;
		}
	}

	control->getToolHandler()->setSelectionEditTools(canChangeColor, canChangeSize, canChangeFill);

	repaintSelection();
}

void XournalView::repaintSelection(bool evenWithoutSelection)
{
	if (evenWithoutSelection)
	{
		gtk_widget_queue_draw(this->widget);
		return;
	}

	EditSelection* selection = getSelection();
	if (selection == nullptr)
	{
		return;
	}

	// repaint always the whole widget
	gtk_widget_queue_draw(this->widget);
}

void XournalView::layoutPages()
{
	Layout* layout = gtk_xournal_get_layout(this->widget);
	layout->recalculate();
}

int XournalView::getDisplayHeight() const
{
	GtkAllocation allocation = {0};
	gtk_widget_get_allocation(this->widget, &allocation);
	return allocation.height;
}

int XournalView::getDisplayWidth() const
{
	GtkAllocation allocation = {0};
	gtk_widget_get_allocation(this->widget, &allocation);
	return allocation.width;
}

bool XournalView::isPageVisible(size_t page, int* visibleHeight)
{
	Rectangle* rect = getVisibleRect(page);
	if (rect)
	{
		if (visibleHeight)
		{
			*visibleHeight = std::lround(rect->height);
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
	size_t p = getCurrentPage();
	if (p == npos || p >= viewPagesLen)
	{
		return false;
	}

	XojPageView* page = viewPages[p];
	return page->cut();
}

bool XournalView::copy()
{
	size_t p = getCurrentPage();
	if (p == npos || p >= viewPagesLen)
	{
		return false;
	}

	XojPageView* page = viewPages[p];
	return page->copy();
}

bool XournalView::paste()
{
	size_t p = getCurrentPage();
	if (p == npos || p >= viewPagesLen)
	{
		return false;
	}

	XojPageView* page = viewPages[p];
	return page->paste();
}

bool XournalView::actionDelete()
{
	size_t p = getCurrentPage();
	if (p == npos || p >= viewPagesLen)
	{
		return false;
	}

	XojPageView* page = viewPages[p];
	return page->actionDelete();
}

Document* XournalView::getDocument()
{
	return control->getDocument();
}

ArrayIterator<XojPageView*> XournalView::pageViewIterator()
{
	return ArrayIterator<XojPageView*>(viewPages, viewPagesLen);
}


XournalppCursor* XournalView::getCursor()
{
	return control->getCursor();
}

EditSelection* XournalView::getSelection()
{
	g_return_val_if_fail(this->widget != nullptr, nullptr);
	g_return_val_if_fail(GTK_IS_XOURNAL(this->widget), nullptr);

	return GTK_XOURNAL(this->widget)->selection;
}
