#include "Redrawable.h"

void Redrawable::redraw(Range & r) {
	redraw(r.getX(), r.getY(), r.getX2(), r.getY2());
}
