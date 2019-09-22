/*
 * Xournal++
 *
 * Draws lined backgrounds of all sorts
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseBackgroundPainter.h"

#include <XournalType.h>

class StavesBackgroundPainter: public BaseBackgroundPainter
{
public:
	StavesBackgroundPainter();
	~StavesBackgroundPainter() override;

public:
	void paint() override;

	/**
	 * Reset all used configuration values
	 */
	void resetConfig() override;


	void paintBackgroundStaves(double offset);

private:
	const double headerSize = 80;
	const double footerSize = 20;
	const double borderSize = 50;

	/**
	 * Distance between lines of staves
	 */
	const double lineDistance = 40;
	/**
	 * Distance between the staves themselves
	 */
	const double staveDistance = 5;
};
