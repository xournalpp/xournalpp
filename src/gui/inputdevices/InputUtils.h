#include "control/ToolHandler.h"
#include "control/settings/Settings.h"
#include "control/settings/SettingsEnums.h"

class InputUtils {
public:
    static void applyButton(ToolHandler* toolHandler, Settings* settings, Buttons button, bool& pressed,
                            bool& toolChanged, bool& configChanged);
};
