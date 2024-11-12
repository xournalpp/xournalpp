#include "PageTypeSelectionMenuBase.h"

#include <algorithm>  // for find_if
#include <iterator>
#include <memory>
#include <vector>

#include <glib-object.h>  // for G_CALLBACK, g_sig...

#include "control/pagetype/PageTypeHandler.h"       // for PageTypeInfo, Pag...
#include "control/settings/PageTemplateSettings.h"  // for PageTemplateSettings
#include "control/settings/Settings.h"              // for Settings
#include "util/raii/GVariantSPtr.h"

namespace {
std::optional<PageType> getInitiallySelectedPageType(const Settings* settings) {
    if (settings) {
        PageTemplateSettings model;
        model.parse(settings->getPageTemplate());
        return model.getPageInsertType();
    }
    return std::nullopt;
}

/**
 * @brief Returns the action target value associated to the given page type.
 */
size_t findIndex(PageTypeHandler* types, const std::optional<PageType>& pt) {
    auto find_index_of = [](const std::vector<std::unique_ptr<PageTypeInfo>>& types, const PageType& pt) {
        auto it = std::find_if(types.begin(), types.end(), [&pt](const auto& info) { return info->page == pt; });
        return it != types.end() ? static_cast<size_t>(std::distance(types.begin(), it)) :
                                   PageTypeSelectionMenuBase::COPY_CURRENT_PLACEHOLDER;
    };

    return pt ? (pt->isSpecial() ?
                         find_index_of(types->getSpecialPageTypes(), pt.value()) + types->getPageTypes().size() :
                         find_index_of(types->getPageTypes(), pt.value())) :
                PageTypeSelectionMenuBase::COPY_CURRENT_PLACEHOLDER;
}

GSimpleAction* createPageTypeSelectionAction(PageTypeHandler* types, const std::optional<PageType>& pt,
                                             const std::string_view& actionName) {
    size_t index = findIndex(types, pt);
    return g_simple_action_new_stateful(actionName.data(), G_VARIANT_TYPE_UINT64, g_variant_new_uint64(index));
}
};  // namespace


PageTypeSelectionMenuBase::PageTypeSelectionMenuBase(PageTypeHandler* typesHandler, const Settings* settings,
                                                     const std::string_view& actionName):
        selectedPT(getInitiallySelectedPageType(settings)),
        typeSelectionAction(createPageTypeSelectionAction(typesHandler, selectedPT, actionName), xoj::util::adopt),
        types(typesHandler) {
    g_signal_connect(G_OBJECT(typeSelectionAction.get()), "change-state", G_CALLBACK(changeSelectionCallback), this);

    // The GAction is not yet added to any GActionMap/GActionGroup - do it in derived classes
}

void PageTypeSelectionMenuBase::changeSelectionCallback(GSimpleAction* ga, GVariant* parameter,
                                                        PageTypeSelectionMenuBase* self) {
    if (g_variant_equal(parameter, xoj::util::GVariantSPtr(g_action_get_state(G_ACTION(ga)), xoj::util::adopt).get())) {
        return;
    }

    g_simple_action_set_state(ga, parameter);
    auto id = g_variant_get_uint64(parameter);
    const auto& types = self->types;
    const auto& genTypes = types->getPageTypes();
    const auto& specialTypes = types->getSpecialPageTypes();

    PageTypeInfo* info = nullptr;
    if (id < genTypes.size()) {
        info = genTypes[id].get();
    } else if (id < genTypes.size() + specialTypes.size()) {
        info = specialTypes[id - genTypes.size()].get();
    }  // The "Copy current page" type has id == npos

    self->selectedPT = info ? std::make_optional(info->page) : std::nullopt;
    self->entrySelected(info);
}

void PageTypeSelectionMenuBase::setSelectedPT(const std::optional<PageType>& selected) {
    if (this->selectedPT != selected) {
        size_t index = findIndex(types, selected);
        g_simple_action_set_state(typeSelectionAction.get(), g_variant_new_uint64(index));
        this->selectedPT = selected;
    }
}
