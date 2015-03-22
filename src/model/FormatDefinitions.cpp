#include "FormatDefinitions.h"

#include <config.h>
#include <glib/gi18n-lib.h>

const FormatUnits XOJ_UNITS[] = {
	{ _("cm"),     28.346 },
	{ _("in"),     72.    },
	{ _("points"), 1.0    }
};

const int XOJ_UNIT_COUNT = sizeof (XOJ_UNITS) / sizeof (FormatUnits);
