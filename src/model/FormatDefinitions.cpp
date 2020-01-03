#include "FormatDefinitions.h"

#include "Util.h"
#include "i18n.h"

const FormatUnits XOJ_UNITS[] = {{"cm", 28.346}, {"in", Util::DPI_NORMALIZATION_FACTOR}, {"points", 1.0}};

const int XOJ_UNIT_COUNT = sizeof(XOJ_UNITS) / sizeof(FormatUnits);
