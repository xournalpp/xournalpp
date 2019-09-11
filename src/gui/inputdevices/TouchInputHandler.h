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

class TouchInputHandler : public AbstractInputHandler
{
private:
	GdkEventSequence* primarySequence = nullptr;
  	GdkEventSequence* secondarySequence = nullptr;
	double priLastPosX = -1.0;
	double priLastPosY = -1.0;
  	double secLastPosX = -1.0;
  	double secLastPosY = -1.0;
  	double lastDiff = 0.0;

private:
	void actionStart(InputEvent* event);
	void actionMotion(InputEvent* event);
	void actionEnd(InputEvent* event);
  void zoomStart(InputEvent* event);
  void zoomMotion(InputEvent* event);
  void zoomEnd(InputEvent* event);

public:
	explicit TouchInputHandler(InputContext* inputContext);
	~TouchInputHandler() override;

	bool handleImpl(InputEvent* event) override;

};


