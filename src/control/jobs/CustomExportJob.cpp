#include "CustomExportJob.h"

#include "control/Control.h"

#include <i18n.h>

CustomExportJob::CustomExportJob(Control* control)
 : BaseExportJob(control, _("Custom Export"))
{
	XOJ_INIT_TYPE(CustomExportJob);
}

CustomExportJob::~CustomExportJob()
{
	XOJ_RELEASE_TYPE(CustomExportJob);
}

