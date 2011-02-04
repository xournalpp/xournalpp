#include "Stroke.h"
#include <gtk/gtk.h>
#include <math.h>
#include <string.h>
#include "../util/ObjectStream.h"

Point::Point() {
	this->x = 0;
	this->y = 0;
	this->z = NO_PRESURE;
}

Point::Point(double x, double y) {
	this->x = x;
	this->y = y;
	this->z = NO_PRESURE;
}

Point::Point(double x, double y, double z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

Stroke::Stroke() :
	Element(ELEMENT_STROKE) {
	width = 0;

	this->pointAllocCount = 0;
	this->pointCount = 0;
	this->points = NULL;
	this->toolType = STROKE_TOOL_PEN;

	this->splitIndex = -1;
	this->copyed = false;
}

Stroke::~Stroke() {
	g_free(this->points);
}

Stroke * Stroke::clone() const {
	Stroke * s = new Stroke();
	s->setColor(this->getColor());
	s->setToolType(this->getToolType());
	s->setWidth(this->getWidth());

	s->allocPointSize(this->pointCount);
	memcpy(s->points, this->points, this->pointCount * sizeof(Point));
	s->pointCount = this->pointCount;

	return s;
}

void Stroke::serialize(ObjectOutputStream & out) {
	out.writeObject("Stroke");

	serializeElement(out);

	out.writeDouble(this->width);

	out.writeInt(this->toolType);

	out.writeData(this->points, this->pointCount, sizeof(Point));

	out.endObject();
}

void Stroke::readSerialized(ObjectInputStream & in) throw (InputStreamException) {
	in.readObject("Stroke");

	readSerializedElement(in);

	this->width = in.readDouble();

	this->toolType = (StrokeTool) in.readInt();

	if (this->points) {
		g_free(this->points);
	}
	this->points = NULL;
	this->pointCount = 0;

	in.readData((void **) &this->points, &this->pointCount);

	in.endObject();
}

void Stroke::setWidth(double width) {
	this->width = width;
}

double Stroke::getWidth() const {
	return width;
}

void Stroke::setCopyed(bool copyed) {
	this->copyed = copyed;
}

bool Stroke::isCopyed() {
	return this->copyed;
}

bool Stroke::isInSelection(ShapeContainer * container) {
	for (int i = 1; i < pointCount; i++) {
		double px = points[i].x;
		double py = points[i].y;

		if (!container->contains(px, py)) {
			return false;
		}
	}

	return true;
}

void Stroke::addPoint(Point p) {
	if (this->pointCount >= this->pointAllocCount) {
		this->allocPointSize(this->pointAllocCount + 100);
	}
	this->points[this->pointCount++] = p;
	sizeCalculated = false;
}

void Stroke::allocPointSize(int size) {
	this->pointAllocCount = size;
	this->points = (Point *) g_realloc(this->points, this->pointAllocCount * sizeof(Point));
}

int Stroke::getPointCount() const {
	return pointCount;
}

ArrayIterator<Point> Stroke::pointIterator() const {
	return ArrayIterator<Point> (points, pointCount);
}

Point Stroke::getPoint(int index) const {
	if (index < 0 || index >= pointCount) {
		g_warning("Stroke::getPoint(%i) out of bounds!", index);
		return Point(0, 0, Point::NO_PRESURE);
	}
	return points[index];
}

const Point * Stroke::getPoints() const {
	return points;
}

void Stroke::freeUnusedPointItems() {
	if (this->pointAllocCount == this->pointCount) {
		return;
	}
	this->pointAllocCount = this->pointCount;
	this->points = (Point *) g_realloc(this->points, this->pointAllocCount * sizeof(Point));
}

void Stroke::setToolType(StrokeTool type) {
	this->toolType = type;
}

StrokeTool Stroke::getToolType() const {
	return toolType;
}

void Stroke::move(double dx, double dy) {
	for (int i = 0; i < pointCount; i++) {
		points[i].x += dx;
		points[i].y += dy;
	}

	this->sizeCalculated = false;
}

void Stroke::scale(double x0, double y0, double fx, double fy) {
	double fz = (fx + fy) / 2;

	for (int i = 0; i < this->pointCount; i++) {
		Point & p = this->points[i];

		p.x -= x0;
		p.x *= fx;
		p.x += x0;

		p.y -= y0;
		p.y *= fy;
		p.y += y0;

		if (p.z != Point::NO_PRESURE) {
			p.z *= fz;
		}
	}
	this->width *= fz;

	this->sizeCalculated = false;
}

bool Stroke::hasPressure() {
	if (this->pointCount > 0) {
		return this->points[0].z != Point::NO_PRESURE;
	}
	return false;
}

void Stroke::scalePressure(double factor) {
	if (!hasPressure()) {
		return;
	}
	for (int i = 0; i < this->pointCount; i++) {
		this->points[i].z *= factor;
	}
}

void Stroke::clearPressure() {
	for (int i = 0; i < this->pointCount; i++) {
		this->points[i].z = Point::NO_PRESURE;
	}
}

void Stroke::setLastPressure(double pressure) {
	if (this->pointCount > 0) {
		this->points[this->pointCount - 1].z = pressure;
	}
}

void Stroke::setPressure(const double * data) {
	if (data == NULL) {
		return;
	}
	for (int i = 0; i < this->pointCount; i++) {
		this->points[i].z = data[i];
	}
}

Stroke * Stroke::splitOnLastIntersects() {
	if (this->splitIndex == -1) {
		// Nothing to split
		return NULL;
	}

	if (splitIndex == this->pointCount - 1) {
		this->pointCount--;
		this->sizeCalculated = false;

		// No new element, only the tail of this is removed
		return NULL;
	}

	if (splitIndex == 0) {
		for (int i = 0; i < this->pointCount - 1; i++) {
			this->points[i] = this->points[i - 1];
		}

		this->sizeCalculated = false;

		// No new element, only the head of this is removed
		return NULL;
	}

	Stroke * s = new Stroke();
	s->setColor(this->getColor());
	s->setToolType(this->getToolType());
	s->setWidth(this->getWidth());
	for (int i = this->splitIndex + 1; i < this->pointCount; i++) {
		s->addPoint(this->points[i]);
	}

	this->pointCount = this->splitIndex;
	this->sizeCalculated = false;

	return s;
}

bool Stroke::intersects(double x, double y, double halfEraserSize, double * gap) {
	if (pointCount < 1) {
		return false;
	}

	double x1 = x - halfEraserSize;
	double x2 = x + halfEraserSize;
	double y1 = y - halfEraserSize;
	double y2 = y + halfEraserSize;

	double lastX = points[0].x;
	double lastY = points[0].y;
	for (int i = 1; i < pointCount; i++) {
		double px = points[i].x;
		double py = points[i].y;

		if (px >= x1 && py >= y1 && px <= x2 && py <= y2) {
			this->splitIndex = i;
			if (gap) {
				*gap = 0;
			}
			return true;
		}

		double len = hypot(px - lastX, py - lastY);
		if (len >= halfEraserSize) {
			/**
			 * The normale to a vector, the padding to a point
			 */
			double p = ABS((x - lastX) * (lastY - py) + (y - lastY) * (px - lastX)) / hypot(lastX - x, lastY - y);

			// The space to the line is in the range, but it can also be parallel
			// and not enough close, so calculate a "circle" with the center on the
			// center of the line


			if (p <= halfEraserSize) {
				double centerX = (lastX + x) / 2;
				double centerY = (lastY + y) / 2;
				double distance = hypot(x - centerX, y - centerY);

				// we should calculate the length of the line within the rectangle, to find out
				// the distance from the border to the point, but the stroken are not rectangular
				// so we can do it simpler
				distance -= hypot((x2 - x1) / 2, (y2 - y1) / 2);

				if (distance <= (len / 2) + 0.1) {
					this->splitIndex = i;

					if (gap) {
						*gap = distance;
					}
					return true;
				}
			}
		}

		lastX = px;
		lastY = py;
	}

	this->splitIndex = -1;
	return false;
}

/**
 * Updates the size
 * The size is needed to only redraw the requestetet part instead of redrawing
 * the whole page (performance reason)
 */
void Stroke::calcSize() {
	if (pointCount == 0) {
		Element::x = 0;
		Element::y = 0;

		// The size of the rectangle, not the size of the pen!
		Element::width = 0;
		Element::height = 0;
	}

	double minX = points[0].x;
	double maxX = points[0].x;
	double minY = points[0].y;
	double maxY = points[0].y;

	for (int i = 1; i < pointCount; i++) {
		if (minX > points[i].x) {
			minX = points[i].x;
		}
		if (maxX < points[i].x) {
			maxX = points[i].x;
		}
		if (minY > points[i].y) {
			minY = points[i].y;
		}
		if (maxY < points[i].y) {
			maxY = points[i].y;
		}
	}

	Element::x = minX - 2;
	Element::y = minY - 2;
	Element::width = maxX - minX + 4 + width;
	Element::height = maxY - minY + 4 + width;
}

