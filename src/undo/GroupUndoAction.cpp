#include "GroupUndoAction.h"

GroupUndoAction::GroupUndoAction()
 : UndoAction("GroupUndoAction")
{
}

GroupUndoAction::~GroupUndoAction()
{
	for (int i = actions.size() - 1; i >= 0; i--)
	{
		delete actions[i];
	}

	actions.clear();
}

void GroupUndoAction::addAction(UndoAction* action)
{
	actions.push_back(action);
}

vector<PageRef> GroupUndoAction::getPages()
{
	vector<PageRef> pages;

	for (UndoAction* a : actions)
	{
		for (PageRef addPage : a->getPages())
		{
			if (!addPage.isValid())
			{
				continue;
			}

			bool pageAlreadyInTheList = false;
			for (PageRef p : pages)
			{
				if (addPage == p)
				{
					pageAlreadyInTheList = true;
					break;
				}
			}

			if (!pageAlreadyInTheList)
			{
				pages.push_back(addPage);
			}
		}
	}

	return pages;
}

bool GroupUndoAction::redo(Control* control)
{
	bool result = true;
	for (size_t i = 0; i < actions.size(); i++)
	{
		result = result && actions[i]->redo(control);
	}

	return result;
}

bool GroupUndoAction::undo(Control* control)
{
	bool result = true;
	for (int i = actions.size() - 1; i >= 0; i--)
	{
		result = result && actions[i]->undo(control);
	}

	return result;
}

string GroupUndoAction::getText()
{
	if (actions.size() == 0)
	{
		return "!! NOTHING !!";
	}

	return actions[0]->getText();
}
