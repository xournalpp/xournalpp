#include "UndoRedoHandler.h"

#include "control/Control.h"

#include "XojMsgBox.h"
#include "config.h"
#include "i18n.h"

#include <algorithm>
#include <cinttypes>

#ifdef UNDO_TRACE

void printAction(UndoAction* action)
{
	if (action) {
		g_message("%" PRIu64 " / %s", (uint64_t) action, action->getClassName());
	} else {
		g_message("(null)");
	}
}

void printUndoList(GList* list)
{
	for (GList* l = list; l != nullptr; l = l->next) {
		UndoAction* action = (UndoAction*) l->data;
		printAction(action);
	}
}

#endif  // UNDO_TRACE

#ifdef UNDO_TRACE
#	define PRINTCONTENTS()               \
		g_message("redoList");            \
		printUndoList(this->redoList);    \
		g_message("undoList");            \
		printUndoList(this->undoList);    \
		g_message("savedUndo");           \
		if (this->savedUndo) {            \
			printAction(this->savedUndo); \
		}
#else
#	define PRINTCONTENTS() (void) 0
#endif  // UNDO_TRACE

UndoRedoHandler::UndoRedoHandler(Control* control): control(control)
{
	XOJ_INIT_TYPE(UndoRedoHandler);
}

UndoRedoHandler::~UndoRedoHandler()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	clearContents();

	XOJ_RELEASE_TYPE(UndoRedoHandler);
}

void UndoRedoHandler::clearContents()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);
#ifdef UNDO_TRACE
	for (auto const& undoAction: this->undoList) {
		g_message("clearContents()::Delete UndoAction: %" PRIu64 " / %s",
		          (size_t) *undoAction,
		          undoAction.getClassName());
	}
#endif  // UNDO_TRACE

	clearRedo();

	this->savedUndo = nullptr;
	this->autosavedUndo = nullptr;

	PRINTCONTENTS();
}

void UndoRedoHandler::clearRedo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);
#ifdef UNDO_TRACE
	for (auto const& undoAction: this->redoList) {
		g_message("clearRedo()::Delete UndoAction: %" PRIu64 " / %s", (size_t) &undoAction, undoAction.getClassName());
	}
#endif
	redoList.clear();
	PRINTCONTENTS();
}

void UndoRedoHandler::undo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (this->undoList.empty()) {
		return;
	}

	g_assert_true(this->undoList.back());

	auto& undoAction = *this->undoList.back();
	this->redoList.emplace_back(std::move(this->undoList.back()));
	this->undoList.pop_back();

	Document* doc = control->getDocument();
	doc->lock();
	bool undoResult = undoAction.undo(this->control);
	doc->unlock();

	if (!undoResult) {
		string msg = FS(_F("Could not undo \"{1}\"\n"
		                   "Something went wrong… Please write a bug report…") %
		                undoAction.getText());
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
	}

	fireUpdateUndoRedoButtons(undoAction.getPages());

	PRINTCONTENTS();
}

void UndoRedoHandler::redo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (this->redoList.empty()) {
		return;
	}

	g_assert_true(this->redoList.back());

	UndoAction& redoAction = *this->redoList.back();

	this->undoList.emplace_back(std::move(this->redoList.back()));
	this->redoList.pop_back();

	Document* doc = control->getDocument();
	doc->lock();
	bool redoResult = redoAction.redo(this->control);
	doc->unlock();

	if (!redoResult) {
		string msg = FS(_F("Could not redo \"{1}\"\n"
		                   "Something went wrong… Please write a bug report…") %
		                redoAction.getText());
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
	}

	fireUpdateUndoRedoButtons(redoAction.getPages());

	PRINTCONTENTS();
}

bool UndoRedoHandler::canUndo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);
	return !this->undoList.empty();
}

bool UndoRedoHandler::canRedo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);
	return !this->redoList.empty();
}

/**
 * Adds an undo Action to the list, or if nullptr does nothing
 */
void UndoRedoHandler::addUndoAction(UndoActionPtr action)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (action == nullptr) {
		return;
	}

	this->undoList.emplace_back(std::move(action));
	clearRedo();
	fireUpdateUndoRedoButtons(this->undoList.back()->getPages());

	PRINTCONTENTS();
}

void UndoRedoHandler::addUndoActionBefore(UndoActionPtr action, UndoAction* before)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	auto iter = std::find_if(begin(this->undoList), end(this->undoList), [before](UndoActionPtr const& smtr_ptr) {
		return (smtr_ptr.get() == before);
	});

	if (iter == end(this->undoList)) {
		addUndoAction(std::move(action));
		return;
	}
	this->undoList.emplace(iter, std::move(action));
	clearRedo();
	fireUpdateUndoRedoButtons(this->undoList.back()->getPages());

	PRINTCONTENTS();
}

bool UndoRedoHandler::removeUndoAction(UndoAction* action)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	auto iter = std::find_if(begin(this->undoList), end(this->undoList), [action](UndoActionPtr const& smtr_ptr) {
		return smtr_ptr.get() == action;
	});
	if (iter == end(this->undoList)) {
		return false;
	}
	this->undoList.erase(iter);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPages());
	return true;
}

string UndoRedoHandler::undoDescription()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (!this->undoList.empty()) {
		UndoAction& a = *this->undoList.back();
		if (!a.getText().empty()) {
			string txt = _("Undo: ");
			txt += a.getText();
			return txt;
		}
	}
	return _("Undo");
}

string UndoRedoHandler::redoDescription()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (!this->redoList.empty()) {
		UndoAction& a = *this->redoList.back();
		if (!a.getText().empty()) {
			string txt = _("Redo: ");
			txt += a.getText();
			return txt;
		}
	}
	return _("Redo");
}

void UndoRedoHandler::fireUpdateUndoRedoButtons(const vector<PageRef>& pages)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	for (auto&& undoRedoListener: this->listener) {
		undoRedoListener->undoRedoChanged();
	}

	for (PageRef page: pages) {
		if (!page.isValid()) {
			continue;
		}

		for (auto&& undoRedoListener: this->listener) {
			undoRedoListener->undoRedoPageChanged(page);
		}
	}
}

void UndoRedoHandler::addUndoRedoListener(UndoRedoListener* listener)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);
	this->listener.emplace_back(listener);
}

bool UndoRedoHandler::isChanged()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (this->undoList.empty()) {
		return this->savedUndo;
	}

	return this->savedUndo != this->undoList.back().get();
}

bool UndoRedoHandler::isChangedAutosave()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (this->undoList.empty()) {
		return this->autosavedUndo;
	}
	return this->autosavedUndo != this->undoList.back().get();
}

void UndoRedoHandler::documentAutosaved()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);
	this->autosavedUndo = this->undoList.empty() ? nullptr : this->undoList.back().get();
}

void UndoRedoHandler::documentSaved()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);
	this->savedUndo = this->undoList.empty() ? nullptr : this->undoList.back().get();
}

const char* UndoRedoHandler::getUndoStackTopTypeName()
{
	if (this->undoList.empty()) {
		return nullptr;
	}
	return this->undoList.back()->getClassName();
}

const char* UndoRedoHandler::getRedoStackTopTypeName()
{
	if (this->redoList.empty()) {
		return nullptr;
	}
	return this->redoList.back()->getClassName();
}
