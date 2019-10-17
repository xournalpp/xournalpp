/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>
#include "AbstractInputHandler.h"
#include <cmath>

class InputContext;

class TouchInputHandler: public AbstractInputHandler
{
private:
	GdkEventSequence* primarySequence = nullptr;
  	GdkEventSequence* secondarySequence = nullptr;

  	double startZoomDistance = 0.0;
  	double lastZoomScrollCenterX = 0.0;
  	double lastZoomScrollCenterY = 0.0;

	double priLastAbsX = -1.0;
	double priLastAbsY = -1.0;
	double secLastAbsX = -1.0;
	double secLastAbsY = -1.0;

	double priLastRelX = -1.0;
	double priLastRelY = -1.0;
	double secLastRelX = -1.0;
	double secLastRelY = -1.0;

private:
	void sequenceStart(InputEvent* event);
	void scrollMotion(InputEvent* event);
	void zoomStart();
	void zoomMotion(InputEvent* event);
	void zoomEnd();

public:
	explicit TouchInputHandler(InputContext* inputContext);
	~TouchInputHandler() override = default;

	bool handleImpl(InputEvent* event) override;

};


