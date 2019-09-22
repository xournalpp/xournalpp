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

#include "model/PageRef.h"

#include <XournalType.h>

class DeleteUndoAction;
class Document;
class EraseUndoAction;
class Layer;
class Range;
class Redrawable;
class Stroke;
class ToolHandler;
class UndoRedoHandler;

class EraseHandler
{
public:
	EraseHandler(UndoRedoHandler* undo, Document* doc, PageRef page, ToolHandler* handler, Redrawable* view);
	virtual ~EraseHandler();

public:
	void erase(double x, double y);
	void finalize();

private:
	void eraseStroke(Layer* l, Stroke* s, double x, double y, Range* range);

private:
	PageRef page;
	ToolHandler* handler;
	Redrawable* view;
	Document* doc;
	UndoRedoHandler* undo;

	DeleteUndoAction* eraseDeleteUndoAction;
	EraseUndoAction* eraseUndoAction;

	double halfEraserSize;
};
