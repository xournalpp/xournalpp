#include "UndoRedoController.h"

#include "Control.h"

#include "gui/XournalView.h"

UndoRedoController::UndoRedoController(Control* control)
 : control(control)
{
	XOJ_INIT_TYPE(UndoRedoController);
}

UndoRedoController::~UndoRedoController()
{
	XOJ_CHECK_TYPE(UndoRedoController);

	this->control = NULL;
	this->layer = NULL;
	elements.clear();

	XOJ_RELEASE_TYPE(UndoRedoController);
}

void UndoRedoController::before()
{
	XOJ_CHECK_TYPE(UndoRedoController);

	EditSelection* selection = control->getWindow()->getXournal()->getSelection();
	if (selection != NULL)
	{
		layer = selection->getSourceLayer();
		for (Element* e: *selection->getElements())
		{
			elements.push_back(e);
		}
	}

	control->clearSelection();
}

void UndoRedoController::after()
{
	XOJ_CHECK_TYPE(UndoRedoController);

	control->resetShapeRecognizer();

	// Restore selection, if any

	if (layer == NULL)
	{
		// No layer - no selection
		return;
	}

	for (Element* e: elements)
	{
		if (layer->indexOf(e) != -1)
		{

		}
	}
}

void UndoRedoController::undo(Control* control)
{
	UndoRedoController handler(control);
	handler.before();

	// Move out of text mode to allow textboxundo to work
	control->clearSelectionEndText();

	control->getUndoRedoHandler()->undo();

	handler.after();
}

void UndoRedoController::redo(Control* control)
{
	UndoRedoController handler(control);
	handler.before();

	control->getUndoRedoHandler()->redo();

	handler.after();
}
