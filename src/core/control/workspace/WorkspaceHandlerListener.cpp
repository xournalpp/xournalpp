#include "WorkspaceHandler.h"
#include "WorkspaceHandlerListener.h"

WorkspaceHandlerListener::WorkspaceHandlerListener(): handler(nullptr) {}
WorkspaceHandlerListener::~WorkspaceHandlerListener() {}

void WorkspaceHandlerListener::registerListener(WorkspaceHandler* handler) {
    this->handler = handler;
    handler->addListener(this);
}
