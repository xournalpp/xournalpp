#include "DocumentListener.h"

#include "model/DocumentChangeType.h"  // for DocumentChangeType

#include "DocumentHandler.h"  // for DocumentHandler

DocumentListener::DocumentListener() = default;

DocumentListener::~DocumentListener() { unregisterListener(); }

void DocumentListener::registerListener(DocumentHandler* handler) {
    this->handler = handler;
    handler->addListener(this);
}

void DocumentListener::unregisterListener() {
    if (this->handler) {
        this->handler->removeListener(this);
    }
}

void DocumentListener::documentChanged(DocumentChangeType type) {}

void DocumentListener::pageSizeChanged(size_t page) {}

void DocumentListener::pageChanged(size_t page) {}

void DocumentListener::pageInserted(size_t page) {}

void DocumentListener::pageDeleted(size_t page) {}

void DocumentListener::pageSelected(size_t page) {}
