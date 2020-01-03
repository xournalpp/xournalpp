#include "EraseUndoAction.h"

#include "gui/Redrawable.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/eraser/EraseableStroke.h"

#include "PageLayerPosEntry.h"
#include "i18n.h"

EraseUndoAction::EraseUndoAction(const PageRef& page): UndoAction("EraseUndoAction") { this->page = page; }

EraseUndoAction::~EraseUndoAction() {
    for (GList* l = this->original; l != nullptr; l = l->next) {
        auto* e = static_cast<PageLayerPosEntry<Stroke>*>(l->data);
        if (!undone) {
            delete e->element;
        }
        delete e;
    }
    g_list_free(this->original);
    this->original = nullptr;

    for (GList* l = this->edited; l != nullptr; l = l->next) {
        auto* e = static_cast<PageLayerPosEntry<Stroke>*>(l->data);
        if (undone) {
            delete e->element;
        }
        delete e;
    }
    g_list_free(this->edited);
    this->edited = nullptr;
}

void EraseUndoAction::addOriginal(Layer* layer, Stroke* element, int pos) {
    this->original = g_list_insert_sorted(this->original, new PageLayerPosEntry<Stroke>(layer, element, pos),
                                          reinterpret_cast<GCompareFunc>(PageLayerPosEntry<Stroke>::cmp));
}

void EraseUndoAction::addEdited(Layer* layer, Stroke* element, int pos) {
    this->edited = g_list_insert_sorted(this->edited, new PageLayerPosEntry<Stroke>(layer, element, pos),
                                        reinterpret_cast<GCompareFunc>(PageLayerPosEntry<Stroke>::cmp));
}

void EraseUndoAction::removeEdited(Stroke* element) {
    for (GList* l = this->edited; l != nullptr; l = l->next) {
        auto* p = static_cast<PageLayerPosEntry<Stroke>*>(l->data);
        if (p->element == element) {
            this->edited = g_list_delete_link(this->edited, l);
            delete p;
            p = nullptr;
            return;
        }
    }
}

void EraseUndoAction::finalize() {
    for (GList* l = this->original; l != nullptr;) {
        auto* p = static_cast<PageLayerPosEntry<Stroke>*>(l->data);
        GList* del = l;
        l = l->next;

        if (p->element->getPointCount() == 0) {
            this->edited = g_list_delete_link(this->edited, del);
            delete p;
            p = nullptr;
        } else {

            // Remove the original and add the copy
            int pos = p->layer->removeElement(p->element, false);

            EraseableStroke* e = p->element->getEraseable();
            GList* stroke = e->getStroke(p->element);
            for (GList* ls = stroke; ls != nullptr; ls = ls->next) {
                auto* copy = static_cast<Stroke*>(ls->data);
                p->layer->insertElement(copy, pos);
                this->addEdited(p->layer, copy, pos);
                pos++;
            }

            delete e;
            e = nullptr;
            p->element->setEraseable(nullptr);
        }
    }

    this->page->firePageChanged();
}

auto EraseUndoAction::getText() -> string { return _("Erase stroke"); }

auto EraseUndoAction::undo(Control* control) -> bool {
    for (GList* l = this->edited; l != nullptr; l = l->next) {
        auto* e = static_cast<PageLayerPosEntry<Stroke>*>(l->data);

        e->layer->removeElement(e->element, false);
        this->page->fireElementChanged(e->element);
    }

    for (GList* l = this->original; l != nullptr; l = l->next) {
        auto* e = static_cast<PageLayerPosEntry<Stroke>*>(l->data);

        e->layer->insertElement(e->element, e->pos);
        this->page->fireElementChanged(e->element);
    }

    this->undone = true;
    return true;
}

auto EraseUndoAction::redo(Control* control) -> bool {
    for (GList* l = this->original; l != nullptr; l = l->next) {
        auto* e = static_cast<PageLayerPosEntry<Stroke>*>(l->data);

        e->layer->removeElement(e->element, false);
        this->page->fireElementChanged(e->element);
    }

    for (GList* l = this->edited; l != nullptr; l = l->next) {
        auto* e = static_cast<PageLayerPosEntry<Stroke>*>(l->data);

        e->layer->insertElement(e->element, e->pos);
        this->page->fireElementChanged(e->element);
    }

    this->undone = false;
    return true;
}
