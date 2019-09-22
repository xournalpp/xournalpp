/*
 * Xournal++
 *
 * Scroll handling for GTK standard implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "ScrollHandling.h"

class ScrollHandlingGtk : public ScrollHandling
{
public:
	ScrollHandlingGtk(GtkScrollable* scrollable);
	virtual ~ScrollHandlingGtk();

public:
	virtual void setLayoutSize(int width, int height);

	int getPreferredWidth() override;
	int getPreferredHeight() override;

	virtual void translate(cairo_t* cr, double& x1, double& x2, double& y1, double& y2);
	virtual void translate(double& x, double& y);

	virtual bool fullRepaint();

	virtual void scrollChanged();

private:
	};
