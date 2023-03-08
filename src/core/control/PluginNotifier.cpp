#include "PluginNotifier.h"

#include "model/Document.h"
#include "model/DocumentListener.h"
#include "plugin/PluginController.h"

PluginNotifier::PluginNotifier(Control* control): control(control) {}

PluginNotifier::~PluginNotifier() = default;


void PluginNotifier::documentChanged(DocumentChangeType type) {
    g_message("Notifier: document changed");
    control->getPluginController()->broadcast("documentChanged");
}

void PluginNotifier::pageSizeChanged(size_t page) {
    g_message("Notifier: size of page %zu changed", page + 1);
    control->getPluginController()->broadcast("pageSizeChanged", page);
}

void PluginNotifier::pageChanged(size_t page) {
    g_message("Notifier: page %zu changed", page + 1);
    control->getPluginController()->broadcast("pageChanged", page);
}

void PluginNotifier::pageInserted(size_t page) {
    g_message("Notifier: page %zu inserted", page + 1);
    control->getPluginController()->broadcast("pageInserted", page);
}

void PluginNotifier::pageDeleted(size_t page) {
    g_message("Notifier: page %zu deleted", page + 1);
    control->getPluginController()->broadcast("pageDeleted", page);
}

void PluginNotifier::pageSelected(size_t page) {  // g_message("page %zu selected", page+1);
}


void PluginNotifier::undoRedoChanged() {  // g_message("undo/redo has changed");
}

void PluginNotifier::undoRedoPageChanged(PageRef page) {
    Document* doc = control->getDocument();
    doc->lock();
    size_t p = doc->indexOf(page);
    doc->unlock();
    g_message("Notifier: undo/redo for page %zu has changed", p + 1);
    control->getPluginController()->broadcast("undoRedoPageChanged", p);
}
