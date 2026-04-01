#include "PageListener.h"

#include "PageHandler.h"

PageListener::PageListener() = default;

PageListener::~PageListener() { unregisterFromHandler(); }

void PageListener::registerToHandler(std::shared_ptr<PageHandler> const& _handler) {
    this->handler = _handler;
    _handler->addListener(this);
}

void PageListener::unregisterFromHandler() {
    if (auto _handler = this->handler.lock(); _handler) {
        _handler->removeListener(this);
        this->handler.reset();
    }
}
