#include "ToolSelectCombocontrol.h"

#include <utility>  // for move

#include "control/ToolEnums.h"
#include "control/actions/ActionDatabase.h"
#include "gui/IconNameHelper.h"
#include "util/i18n.h"

static std::vector<ComboToolButton::Entry> makeEntries(IconNameHelper& icons, bool hideAudio) {
    using Entry = ComboToolButton::Entry;
    std::vector<Entry> res = {Entry(_("Select Rectangle"), icons.iconName("select-rect"), TOOL_SELECT_RECT),
                              Entry(_("Select Region"), icons.iconName("select-lasso"), TOOL_SELECT_REGION),
                              Entry(_("Select Multi-Layer Rectangle"), icons.iconName("select-multilayer-rect"),
                                    TOOL_SELECT_MULTILAYER_RECT),
                              Entry(_("Select Multi-Layer Region"), icons.iconName("select-multilayer-lasso"),
                                    TOOL_SELECT_MULTILAYER_REGION),
                              Entry(_("Select Object"), icons.iconName("object-select"), TOOL_SELECT_OBJECT),
                              Entry(_("Play Object"), icons.iconName("object-play"), TOOL_PLAY_OBJECT)};
    if (hideAudio) {
        res.pop_back();
    }
    return res;
}

ToolSelectCombocontrol::ToolSelectCombocontrol(std::string id, IconNameHelper& icons, const ActionDatabase& db,
                                               bool hideAudio):
        ComboToolButton(std::move(id), Category::TOOLS, icons.iconName("combo-selection"), _("Selection dropdown menu"),
                        makeEntries(icons, hideAudio), db.getAction(Action::SELECT_TOOL)) {}
