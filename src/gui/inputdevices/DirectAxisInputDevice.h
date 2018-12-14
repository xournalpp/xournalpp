/*
 * Xournal++
 *
 * Base class for device input handling
 * This class axes the axes direct, which prevents crashes on some X11 systems
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseInputDevice.h"

class DirectAxisInputDevice : public BaseInputDevice
{
public:
	DirectAxisInputDevice(GtkWidget* widget, XournalView* view);
	virtual ~DirectAxisInputDevice();

protected:
	/**
	 * Read Pressure over GTK
	 */
	bool getPressureMultiplier(GdkEvent* event, double& presure);

private:
	XOJ_TYPE_ATTRIB;
};
