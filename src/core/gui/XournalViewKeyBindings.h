/*
 * Xournal++
 *
 * Key bindings for text editor event handling
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gdk/gdkkeysyms.h>

#include "control/KeyBindingsGroup.h"

/*
 * TODO
 * Before this PR: GDK_KEY_KP_Up did ScrollHandler::scrollByOnePage and SHIFT + GDK_KEY_KP_Up did nothing
 * and similarly for other directions
 * I changed that to: GDK_KEY_KP_Up does as GDK_KEY_Up and SHIFT + GDK_KEY_KP_Up does ScrollHandler::scrollByOnePage
 *
 * This is up for debate
 */

using KeyBindingsUtil::wrap;

// clang-format off
const KeyBindingsGroup<ScrollHandler> navigationKeyBindings({
        {{NONE, GDK_KEY_Page_Down},    wrap<&ScrollHandler::goToNextPage>},
        {{NONE, GDK_KEY_KP_Page_Down}, wrap<&ScrollHandler::goToNextPage>},
        {{NONE, GDK_KEY_Page_Up},      wrap<&ScrollHandler::goToPreviousPage>},
        {{NONE, GDK_KEY_KP_Page_Up},   wrap<&ScrollHandler::goToPreviousPage>},
        {{NONE, GDK_KEY_space},        wrap<&ScrollHandler::scrollByVisibleArea, ScrollHandler::DOWN>},
        {{SHIFT, GDK_KEY_space},       wrap<&ScrollHandler::scrollByVisibleArea, ScrollHandler::UP>},

        {{SHIFT, GDK_KEY_KP_Up},       wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::UP>},
        {{SHIFT, GDK_KEY_Up},          wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::UP>},
        {{SHIFT, GDK_KEY_k},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::UP>},  // Why??
        {{SHIFT, GDK_KEY_K},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::UP>},  // Why??
        {{SHIFT, GDK_KEY_KP_Down},     wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::DOWN>},
        {{SHIFT, GDK_KEY_Down},        wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::DOWN>},
        {{SHIFT, GDK_KEY_j},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::DOWN>},  // Why??
        {{SHIFT, GDK_KEY_J},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::DOWN>},  // Why??
        {{SHIFT, GDK_KEY_KP_Left},     wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::LEFT>},
        {{SHIFT, GDK_KEY_Left},        wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::LEFT>},
        {{SHIFT, GDK_KEY_h},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::LEFT>},  // Why??
        {{SHIFT, GDK_KEY_KP_Right},    wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::RIGHT>},
        {{SHIFT, GDK_KEY_Right},       wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::RIGHT>},
        {{SHIFT, GDK_KEY_l},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::RIGHT>},  // Why??

        {{NONE, GDK_KEY_KP_Up},        wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::UP>},
        {{NONE, GDK_KEY_Up},           wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::UP>},
        {{NONE, GDK_KEY_k},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::UP>},  // Why??
        {{NONE, GDK_KEY_K},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::UP>},  // Why??
        {{NONE, GDK_KEY_KP_Down},      wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::DOWN>},
        {{NONE, GDK_KEY_Down},         wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::DOWN>},
        {{NONE, GDK_KEY_j},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::DOWN>},  // Why??
        {{NONE, GDK_KEY_J},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::DOWN>},  // Why??
        {{NONE, GDK_KEY_KP_Left},      wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::LEFT>},
        {{NONE, GDK_KEY_Left},         wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::LEFT>},
        {{NONE, GDK_KEY_h},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::LEFT>},  // Why??
        {{NONE, GDK_KEY_KP_Right},     wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::RIGHT>},
        {{NONE, GDK_KEY_Right},        wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::RIGHT>},
        {{NONE, GDK_KEY_l},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::RIGHT>},  // Why??

        {{NONE, GDK_KEY_End},          wrap<&ScrollHandler::goToLastPage>},
        {{NONE, GDK_KEY_KP_End},       wrap<&ScrollHandler::goToLastPage>},
        {{NONE, GDK_KEY_Home},         wrap<&ScrollHandler::goToFirstPage>},
        {{NONE, GDK_KEY_KP_Home},      wrap<&ScrollHandler::goToFirstPage>}
});
// clang-format on

template <size_t n>
void setColorByNumber(Control* ctrl) {
    if (auto* db = ctrl->getActionDatabase(); db->isActionEnabled(Action::TOOL_COLOR)) {
        const auto& colors = ctrl->getWindow()->getToolMenuHandler()->getColorToolItems();
        if (n < colors.size()) {
            db->fireChangeActionState(Action::TOOL_COLOR, colors[n]->getColor());
        }
    }
}

const KeyBindingsGroup<Control> colorsKeyBindings({
        {{NONE, GDK_KEY_1}, setColorByNumber<0>},
        {{NONE, GDK_KEY_2}, setColorByNumber<1>},
        {{NONE, GDK_KEY_3}, setColorByNumber<2>},
        {{NONE, GDK_KEY_4}, setColorByNumber<3>},
        {{NONE, GDK_KEY_5}, setColorByNumber<4>},
        {{NONE, GDK_KEY_6}, setColorByNumber<5>},
        {{NONE, GDK_KEY_7}, setColorByNumber<6>},
        {{NONE, GDK_KEY_8}, setColorByNumber<7>},
        {{NONE, GDK_KEY_9}, setColorByNumber<8>},
        {{NONE, GDK_KEY_0}, setColorByNumber<9>},
});
