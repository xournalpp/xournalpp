#include "RepaintWidgetHandler.h"


RepaintWidgetHandler::RepaintWidgetHandler(GtkWidget * width)
{
	XOJ_INIT_TYPE(RepaintWidgetHandler);

	g_mutex_init(&this->mutex);
	this->widget = width;
	this->complete = false;
	this->rescaleId = 0;
}

RepaintWidgetHandler::~RepaintWidgetHandler()
{
	XOJ_CHECK_TYPE(RepaintWidgetHandler);

	for (Rectangle* r : this->rects)
	{
		delete r;
	}
	this->rects.clear();

	XOJ_RELEASE_TYPE(RepaintWidgetHandler);
}

void RepaintWidgetHandler::repaintComplete()
{
	XOJ_CHECK_TYPE(RepaintWidgetHandler);

	g_mutex_lock(&this->mutex);
	this->complete = true;

	for (Rectangle* r : this->rects)
	{
		delete r;
	}
	this->rects.clear();

	addRepaintCallback();

	g_mutex_unlock(&this->mutex);
}

void RepaintWidgetHandler::repaintRects(Rectangle* rect)
{
	XOJ_CHECK_TYPE(RepaintWidgetHandler);

	g_mutex_lock(&this->mutex);
	if (this->complete)
	{
		delete rect;
		rect = NULL;
	}
	else
	{
		this->rects.push_front(rect);
	}
	addRepaintCallback();

	g_mutex_unlock(&this->mutex);
}

bool RepaintWidgetHandler::idleRepaint(RepaintWidgetHandler * data)
{
	XOJ_CHECK_TYPE_OBJ(data, RepaintWidgetHandler);

	g_mutex_lock(&data->mutex);
	bool complete = data->complete;
	std::list<Rectangle*> rects = data->rects;

	data->rects.clear();
	data->complete = false;
	data->rescaleId = 0;

	g_mutex_unlock(&data->mutex);

	gdk_threads_enter();

	gtk_widget_queue_draw(data->widget);

	if (complete)
	{
		//gtk_widget_queue_draw(data->widget);
	}
	else
	{
		for (Rectangle* r : rects)
		{
			delete r;
			r = NULL;
		}
		rects.clear();
	}

	gdk_flush();

	gdk_threads_leave();

	// do not call again
	return false;
}

void RepaintWidgetHandler::addRepaintCallback()
{
	XOJ_CHECK_TYPE(RepaintWidgetHandler);

	if (this->rescaleId)
	{
		return;
	}

	this->rescaleId = g_idle_add((GSourceFunc) idleRepaint, this);
}

