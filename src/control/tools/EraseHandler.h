/*
 * Xournal++
 *
 * Handles the erase of stroke, in special split into different parts etc.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>
#include "../../model/PageRef.h"

class DeleteUndoAction;
class EraseUndoAction;
class Layer;
class Stroke;
class ToolHandler;
class Redrawable;
class Document;
class UndoRedoHandler;
class Range;

class EraseHandler
{
public:
	EraseHandler(UndoRedoHandler* undo, Document* doc, PageRef page,
				ToolHandler* handler, Redrawable* view);
	virtual ~EraseHandler();

public:
	void erase(double x, double y);
	void finalize();

private:
	void eraseStroke(Layer* l, Stroke* s, double x, double y, Range* range);

private:
	XOJ_TYPE_ATTRIB;


	PageRef page;
	ToolHandler* handler;
	Redrawable* view;
	Document* doc;
	UndoRedoHandler* undo;

	DeleteUndoAction* eraseDeleteUndoAction;
	EraseUndoAction* eraseUndoAction;

	double halfEraserSize;
};
