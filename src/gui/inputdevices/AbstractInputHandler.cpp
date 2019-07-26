//
// Created by ulrich on 06.04.19.
//

#include "AbstractInputHandler.h"
#include "InputContext.h"
#include <gui/XournalppCursor.h>

AbstractInputHandler::AbstractInputHandler(InputContext* inputContext)
{
	XOJ_INIT_TYPE(AbstractInputHandler);

	this->inputContext = inputContext;
}

AbstractInputHandler::~AbstractInputHandler()
{
	XOJ_CHECK_TYPE(AbstractInputHandler);

	XOJ_RELEASE_TYPE(AbstractInputHandler);
}

void AbstractInputHandler::block(bool block)
{
	XOJ_CHECK_TYPE(AbstractInputHandler);

	this->blocked = block;
	this->onBlock();
}

bool AbstractInputHandler::isBlocked()
{
	XOJ_CHECK_TYPE(AbstractInputHandler);

	return this->blocked;
}

bool AbstractInputHandler::handle(InputEvent* event)
{
	XOJ_CHECK_TYPE(AbstractInputHandler);

	if (!this->blocked)
	{
		this->inputContext->getXournal()->view->getCursor()->setInputDeviceClass(event->deviceClass);
		return this->handleImpl(event);
	} else {
		return true;
	}
}

/**
 * Get Page at current position
 *
 * @return page or NULL if none
 */
XojPageView* AbstractInputHandler::getPageAtCurrentPosition(InputEvent* event)
{
	XOJ_CHECK_TYPE(AbstractInputHandler);

	if (event == nullptr)
	{
		return nullptr;
	}

	gdouble eventX = event->relativeX;
	gdouble eventY = event->relativeY;

	//take scroll offset into account
	this->inputContext->getScrollHandling()->translate(eventX, eventY);

	GtkXournal* xournal = this->inputContext->getXournal();

	double x = eventX + xournal->x;
	double y = eventY + xournal->y;

	return xournal->layout->getViewAt(x,y);
}

/**
 * Get input data relative to current input page
 */
PositionInputData AbstractInputHandler::getInputDataRelativeToCurrentPage(XojPageView* page, InputEvent* event)
{
	XOJ_CHECK_TYPE(AbstractInputHandler);

	GtkXournal* xournal = inputContext->getXournal();

	gdouble eventX = event->relativeX;
	gdouble eventY = event->relativeY;

	//take scroll offset into account
	this->inputContext->getScrollHandling()->translate(eventX, eventY);

	PositionInputData pos = {};
	pos.x = eventX - page->getX() - xournal->x;
	pos.y = eventY - page->getY() - xournal->y;
	pos.pressure = Point::NO_PRESSURE;

	if (this->inputContext->getSettings()->isPressureSensitivity())
	{
		pos.pressure = event->pressure;
	}

	pos.state = this->inputContext->getModifierState();
	pos.timestamp = event->timestamp;

	return pos;
}

void AbstractInputHandler::onBlock()
{
	XOJ_CHECK_TYPE(AbstractInputHandler);
}

void AbstractInputHandler::handleScrollEvent(InputEvent* event)
{
    XOJ_CHECK_TYPE(AbstractInputHandler);

    // use root coordinates as reference point because
    // scrolling changes window relative coordinates
    // see github Gnome/evince@1adce5486b10e763bed869

    // GTK handles event compression/filtering differently between versions - this may be needed on certain hardware/GTK combinations.
    if (std::abs((double)(this->scrollStartX - event->absoluteX)) < 0.1 && std::abs((double)(this->scrollStartY - event->absoluteY)) < 0.1 )
    {
        return;
    }

    if (this->scrollOffsetX == 0 && this->scrollOffsetY == 0)
    {
        this->scrollOffsetX = this->scrollStartX - event->absoluteX;
        this->scrollOffsetY = this->scrollStartY - event->absoluteY;

        Util::execInUiThread([&]() {
            this->inputContext->getXournal()->layout->scrollRelative(this->scrollOffsetX, this->scrollOffsetY);

            // Scrolling done, so reset our counters
            this->scrollOffsetX = 0;
            this->scrollOffsetY = 0;
        });

        // Update the reference for the scroll-offset
        this->scrollStartX = event->absoluteX;
        this->scrollStartY = event->absoluteY;
    }
}
