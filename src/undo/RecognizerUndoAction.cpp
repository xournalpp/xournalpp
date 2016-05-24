#include "RecognizerUndoAction.h"

#include "gui/Redrawable.h"
#include "model/Layer.h"
#include "model/Stroke.h"

#include <i18n.h>
#include <Stacktrace.h>

RecognizerUndoAction::RecognizerUndoAction(PageRef page, Layer* layer, Stroke* original, Stroke* recognized) :
		UndoAction("RecognizerUndoAction")
{
	XOJ_INIT_TYPE(RecognizerUndoAction);

	this->page = page;
	this->layer = layer;
	this->recognized = recognized;

	addSourceElement(original);
}

RecognizerUndoAction::~RecognizerUndoAction()
{
	XOJ_CHECK_TYPE(RecognizerUndoAction);

	if (this->undone)
	{
		delete this->recognized;
	}
	else
	{
		for (Stroke* s : this->original)
		{
			delete s;
		}
	}
	this->recognized = NULL;

	XOJ_RELEASE_TYPE(RecognizerUndoAction);
}

void RecognizerUndoAction::addSourceElement(Stroke* s)
{
	XOJ_CHECK_TYPE(RecognizerUndoAction);
	
	for (Stroke* s2 : this->original)
	{
		if (s2 == s)
		{
			g_warning("RecognizerUndoAction::addSourceElement() twice the same\n");
			Stacktrace::printStracktrace();
			return;
		}
	}

	this->original.push_back(s);
}

bool RecognizerUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(RecognizerUndoAction);

	int pos = this->layer->removeElement(this->recognized, false);
	this->page->fireElementChanged(this->recognized);

	int i = 0;
	for (Stroke* s : this->original)
	{
		this->layer->insertElement(s, pos);
		this->page->fireElementChanged(s);
		i++;
	}

	this->undone = true;
	return true;
}

bool RecognizerUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(RecognizerUndoAction);

	int pos = 0;
	for (Stroke* s : this->original)
	{
		pos = this->layer->removeElement(s, false);
		this->page->fireElementChanged(s);
	}
	this->layer->insertElement(this->recognized, pos);

	this->page->fireElementChanged(this->recognized);

	this->undone = false;
	return true;
}

string RecognizerUndoAction::getText()
{
	XOJ_CHECK_TYPE(RecognizerUndoAction);

	return _("Stroke recognizer");
}

