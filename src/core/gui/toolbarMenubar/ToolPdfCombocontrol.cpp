#include "ToolPdfCombocontrol.h"

#include <utility>  // for move

#include "control/ToolEnums.h"
#include "control/actions/ActionDatabase.h"
#include "gui/IconNameHelper.h"
#include "util/i18n.h"

ToolPdfCombocontrol::ToolPdfCombocontrol(std::string id, IconNameHelper& icons, const ActionDatabase& db):
        ComboToolButton(
                std::move(id), icons.iconName("select-pdf-text-ht"), _("PDF text selection dropdown menu"),
                {Entry(_("Select Linear PDF Text"), icons.iconName("select-pdf-text-ht"), TOOL_SELECT_PDF_TEXT_LINEAR),
                 Entry(_("Select PDF Text In Rectangle"), icons.iconName("select-pdf-text-area"),
                       TOOL_SELECT_PDF_TEXT_RECT)},
                db.getAction(Action::SELECT_TOOL)) {}
