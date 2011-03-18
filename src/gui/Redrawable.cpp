#include "Redrawable.h"

void Redrawable::repaint(Range & r) {
	repaint(r.getX(), r.getY(), r.getX2(), r.getY2());
}
