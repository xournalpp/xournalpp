#include "PageListener.h"

#include "PageHandler.h"

PageListener::PageListener() = default;

PageListener::~PageListener() { unregisterListener(); }

void PageListener::registerListener(PageHandler* handler) {
    this->handler = handler;
    handler->addListener(this);
}

void PageListener::unregisterListener() {
    if (this->handler) {
        this->handler->removeListener(this);
        this->handler = nullptr;
    }
}
