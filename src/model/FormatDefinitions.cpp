#include "FormatDefinitions.h"

#include <i18n.h>

const FormatUnits XOJ_UNITS[] = {
	{ _C("cm"),     28.346 },
	{ _C("in"),     72.    },
	{ _C("points"), 1.0    }
};

const int XOJ_UNIT_COUNT = sizeof(XOJ_UNITS) / sizeof(FormatUnits);
