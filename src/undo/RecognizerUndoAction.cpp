#include "RecognizerUndoAction.h"

#include "gui/Redrawable.h"
#include "model/Layer.h"
#include "model/Stroke.h"

#include <i18n.h>
#include <Stacktrace.h>

RecognizerUndoAction::RecognizerUndoAction(PageRef page, Layer* layer, Stroke* original, Stroke* recognized)
 : UndoAction("RecognizerUndoAction")
{
	this->page = page;
	this->layer = layer;
	this->recognized = recognized;

	addSourceElement(original);
}

RecognizerUndoAction::~RecognizerUndoAction()
{
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
	this->recognized = nullptr;
	this->original.clear();
}

void RecognizerUndoAction::addSourceElement(Stroke* s)
{
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
	return _("Stroke recognizer");
}

