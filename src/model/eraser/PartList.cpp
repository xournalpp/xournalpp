#include "PartList.h"

#include "EraseableStrokePart.h"

PartList::PartList() = default;

PartList::~PartList() {
    for (GList* l = this->data; l != nullptr; l = l->next) {
        auto* p = static_cast<EraseableStrokePart*>(l->data);
        delete p;
    }
    g_list_free(this->data);
    this->data = nullptr;
}

void PartList::add(EraseableStrokePart* part) { this->data = g_list_append(this->data, part); }

auto PartList::clone() -> PartList* {
    auto* list = new PartList();
    for (GList* l = this->data; l != nullptr; l = l->next) {
        auto* p = static_cast<EraseableStrokePart*>(l->data);
        list->data = g_list_append(list->data, p->clone());
    }

    return list;
}
