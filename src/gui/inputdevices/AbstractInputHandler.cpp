//
// Created by ulrich on 06.04.19.
//

#include "AbstractInputHandler.h"
#include "InputContext.h"
#include <gui/XournalppCursor.h>

AbstractInputHandler::AbstractInputHandler(InputContext* inputContext)
{
	this->inputContext = inputContext;
}

AbstractInputHandler::~AbstractInputHandler()
{
}

void AbstractInputHandler::block(bool block)
{
	this->blocked = block;
	this->onBlock();
}

bool AbstractInputHandler::isBlocked()
{
	return this->blocked;
}

bool AbstractInputHandler::handle(InputEvent* event)
{
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
 * @return page or nullptr if none
 */
XojPageView* AbstractInputHandler::getPageAtCurrentPosition(InputEvent* event)
{
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
}
