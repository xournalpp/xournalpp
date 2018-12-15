/*
 * Xournal++
 *
 * Base class for device input handling
 * Data to do an input
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

typedef struct {
	double x;
	double y;
	double pressure;

	/**
	 * State flags from GDKevent (Shift down etc.)
	 */
	int state;
} PositionInputData;
 
