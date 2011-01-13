#include "FormatDefinitions.h"
#include "../gettext.h"

FormatUnits XOJ_UNITS[] = {
		{ _("cm"),     28.346 },
		{ _("in"),     72. },
		{ _("pixels"), 72. / DISPLAY_DPI_DEFAULT },
		{ _("points"), 1.0 }
};

const int XOJ_UNIT_COUNT = sizeof(XOJ_UNITS) / sizeof(FormatUnits);

PaperFormat XOJ_FORMATS[] = {
		{ _("A4"), 595.27, 841.89 },
		{ _("Letter"), 612.00, 792.00 },
		{ _("Custom"), -1, -1 }
};

const int XOJ_FORMAT_COUNT = sizeof(XOJ_FORMATS) / sizeof(PaperFormat);
const int XOJ_FORMAT_CUSTOM_ID = XOJ_FORMAT_COUNT - 1;

