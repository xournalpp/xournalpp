#include "Stroke.h"

#include <serializing/ObjectInputStream.h>
#include <serializing/ObjectOutputStream.h>

#include <i18n.h>

#include <cmath>

Stroke::Stroke()
 : AudioElement(ELEMENT_STROKE)
{
}

Stroke::~Stroke()
{
	g_free(this->points);
	this->points = NULL;
	this->pointCount = 0;
	this->pointAllocCount = 0;
}

/**
 * Clone style attributes, but not the data (position, width etc.)
 */
void Stroke::applyStyleFrom(const Stroke* other)
{
	setColor(other->getColor());
	setToolType(other->getToolType());
	setWidth(other->getWidth());
	setFill(other->getFill());
	setLineStyle(other->getLineStyle());

	cloneAudioData(other);
}

Stroke* Stroke::cloneStroke() const
{
	Stroke* s = new Stroke();
	s->applyStyleFrom(this);

	s->allocPointSize(this->pointCount);
	memcpy(s->points, this->points, this->pointCount * sizeof(Point));
	s->pointCount = this->pointCount;

	return s;
}

Element* Stroke::clone()
{
	return this->cloneStroke();
}

void Stroke::serialize(ObjectOutputStream& out)
{
	out.writeObject("Stroke");

	serializeAudioElement(out);

	out.writeDouble(this->width);

	out.writeInt(this->toolType);

	out.writeInt(fill);

	out.writeData(this->points, this->pointCount, sizeof(Point));

	this->lineStyle.serialize(out);

	out.endObject();
}

void Stroke::readSerialized(ObjectInputStream& in)
{
	in.readObject("Stroke");

	readSerializedAudioElement(in);

	this->width = in.readDouble();

	this->toolType = (StrokeTool) in.readInt();

	this->fill = in.readInt();

	g_free(this->points);
	this->points = NULL;
	this->pointCount = 0;
	in.readData((void**) &this->points, &this->pointCount);

	this->lineStyle.readSerialized(in);

	in.endObject();
}

/**
 * Option to fill the shape:
 *  -1: The shape is not filled
 * 255: The shape is fully opaque filled
 * ...
 *   1: The shape is nearly fully transparent filled
 */
int Stroke::getFill() const
{
	return fill;
}

/**
 * Option to fill the shape:
 *  -1: The shape is not filled
 * 255: The shape is fully opaque filled
 * ...
 *   1: The shape is nearly fully transparent filled
 */
void Stroke::setFill(int fill)
{
	this->fill = fill;
}

void Stroke::setWidth(double width)
{
	this->width = width;
}

double Stroke::getWidth() const
{
	return this->width;
}

bool Stroke::isInSelection(ShapeContainer* container)
{
	for (int i = 0; i < this->pointCount; i++)
	{
		double px = this->points[i].x;
		double py = this->points[i].y;

		if (!container->contains(px, py))
		{
			return false;
		}
	}

	return true;
}

void Stroke::setFirstPoint(double x, double y)
{
	if (this->pointCount > 0)
	{
		Point& p = this->points[0];
		p.x = x;
		p.y = y;
		this->sizeCalculated = false;
	}
}

void Stroke::setLastPoint(double x, double y)
{
	if (this->pointCount > 0)
	{
		Point& p = this->points[this->pointCount - 1];
		p.x = x;
		p.y = y;
		this->sizeCalculated = false;
	}
}

void Stroke::setLastPoint(Point p)
{
	setLastPoint(p.x, p.y);
}

void Stroke::addPoint(Point p)
{
	if (this->pointCount >= this->pointAllocCount - 1)
	{
		this->allocPointSize(this->pointAllocCount + 100);
	}
	this->points[this->pointCount++] = p;
	this->sizeCalculated = false;
}

void Stroke::allocPointSize(int size)
{
	this->pointAllocCount = size;
	this->points = (Point*) g_realloc(this->points, this->pointAllocCount * sizeof(Point));
}

int Stroke::getPointCount() const
{
	return this->pointCount;
}

ArrayIterator<Point> Stroke::pointIterator() const
{
	return ArrayIterator<Point> (points, pointCount);
}

void Stroke::deletePointsFrom(int index)
{
	if (this->pointCount <= index)
	{
		return;
	}
	this->pointCount = index;
}

void Stroke::deletePoint(int index)
{
	if (this->pointCount <= index)
	{
		return;
	}

	for (int i = 0; i < this->pointCount; i++)
	{
		if (i >= index)
		{
			this->points[i] = this->points[i + 1];
		}
	}
	this->pointCount--;
}

Point Stroke::getPoint(int index) const
{
	if (index < 0 || index >= pointCount)
	{
		g_warning("Stroke::getPoint(%i) out of bounds!", index);
		return Point(0, 0, Point::NO_PRESSURE);
	}
	return points[index];
}

const Point* Stroke::getPoints() const
{
	return this->points;
}

void Stroke::freeUnusedPointItems()
{
	if (this->pointAllocCount == this->pointCount)
	{
		return;
	}
	this->pointAllocCount = this->pointCount + 1;
	this->points = (Point*) g_realloc(this->points, this->pointAllocCount * sizeof(Point));
}

void Stroke::setToolType(StrokeTool type)
{
	this->toolType = type;
}

StrokeTool Stroke::getToolType() const
{
	return this->toolType;
}

void Stroke::setLineStyle(const LineStyle& style)
{
	this->lineStyle = style;
}

const LineStyle& Stroke::getLineStyle() const
{
	return this->lineStyle;
}

void Stroke::move(double dx, double dy)
{
	for (int i = 0; i < pointCount; i++)
	{
		points[i].x += dx;
		points[i].y += dy;
	}

	this->sizeCalculated = false;
}

void Stroke::rotate(double x0, double y0, double xo, double yo, double th)
{
	for (int i = 0; i < this->pointCount; i++)
	{
		Point& p = this->points[i];

		p.x -= x0;	//move to origin
		p.y -= y0;
		double offset = 0.7; // __DBL_EPSILON__;
		p.x -= xo-offset;	//center to origin
		p.y -= yo-offset;

		double x1 = p.x * cos(th) - p.y * sin(th); 	
		double y1 = p.y * cos(th) + p.x * sin(th);	
		p.x = x1;
		p.y = y1;

		p.x += x0;	//restore the position
		p.y += y0;

		p.x += xo-offset;	//center it
		p.y += yo-offset;		
	}
	//Width and Height will likely be changed after this operation
	calcSize();
}

void Stroke::scale(double x0, double y0, double fx, double fy)
{
	double fz = sqrt(fx * fy);

	for (int i = 0; i < this->pointCount; i++)
	{
		Point& p = this->points[i];
		
		p.x -= x0;
		p.x *= fx;
		p.x += x0;

		p.y -= y0;
		p.y *= fy;
		p.y += y0;

		if (p.z != Point::NO_PRESSURE)
		{
			p.z *= fz;
		}
	}
	this->width *= fz;

	this->sizeCalculated = false;
}

bool Stroke::hasPressure() const
{
	if (this->pointCount > 0)
	{
		return this->points[0].z != Point::NO_PRESSURE;
	}
	return false;
}

double Stroke::getAvgPressure() const
{
	double summatory = 0;
	for (int i = 0; i < this->pointCount; i++)
	{
		summatory += this->points[i].z;
	}
	return summatory / this->pointCount;
}

void Stroke::scalePressure(double factor)
{
	if (!hasPressure())
	{
		return;
	}
	for (int i = 0; i < this->pointCount; i++)
	{
		this->points[i].z *= factor;
	}
}

void Stroke::clearPressure()
{
	for (int i = 0; i < this->pointCount; i++)
	{
		this->points[i].z = Point::NO_PRESSURE;
	}
}

void Stroke::setLastPressure(double pressure)
{
	if (this->pointCount > 0)
	{
		this->points[this->pointCount - 1].z = pressure;
	}
}

void Stroke::setPressure(const vector<double>& pressure)
{
	// The last pressure is not used - as there is no line drawn from this point
	if (this->pointCount - 1 > (int)pressure.size())
	{
		g_warning("invalid pressure point count: %i, expected %i", (int)pressure.size(), (int)this->pointCount - 1);
		return;
	}

	for (int i = 0; i < this->pointCount && i < (int)pressure.size(); i++)
	{
		this->points[i].z = pressure[i];
	}
}

/**
 * split index is the split point, minimimum is 1 NOT 0
 */
bool Stroke::intersects(double x, double y, double halfEraserSize)
{
	return intersects(x, y, halfEraserSize, nullptr);
}

/**
 * split index is the split point, minimimum is 1 NOT 0
 */
bool Stroke::intersects(double x, double y, double halfEraserSize, double* gap)
{
	if (this->pointCount < 1)
	{
		return false;
	}

	double x1 = x - halfEraserSize;
	double x2 = x + halfEraserSize;
	double y1 = y - halfEraserSize;
	double y2 = y + halfEraserSize;

	double lastX = points[0].x;
	double lastY = points[0].y;
	for (int i = 1; i < pointCount; i++)
	{
		double px = points[i].x;
		double py = points[i].y;

		if (px >= x1 && py >= y1 && px <= x2 && py <= y2)
		{
			if (gap)
			{
				*gap = 0;
			}
			return true;
		}

		double len = hypot(px - lastX, py - lastY);
		if (len >= halfEraserSize)
		{
			/**
			 * The normale to a vector, the padding to a point
			 */
			double p = ABS((x - lastX) * (lastY - py) + (y - lastY) * (px - lastX)) / hypot(lastX - x, lastY - y);

			// The space to the line is in the range, but it can also be parallel
			// and not enough close, so calculate a "circle" with the center on the
			// center of the line

			if (p <= halfEraserSize)
			{
				double centerX = (lastX + x) / 2;
				double centerY = (lastY + y) / 2;
				double distance = hypot(x - centerX, y - centerY);

				// we should calculate the length of the line within the rectangle, to find out
				// the distance from the border to the point, but the stroken are not rectangular
				// so we can do it simpler
				distance -= hypot((x2 - x1) / 2, (y2 - y1) / 2);

				if (distance <= (len / 2) + 0.1)
				{
					if (gap)
					{
						*gap = distance;
					}
					return true;
				}
			}
		}

		lastX = px;
		lastY = py;
	}

	return false;
}

/**
 * Updates the size
 * The size is needed to only redraw the requested part instead of redrawing
 * the whole page (performance reason).
 * Also used for Selected Bounding box.
 */
void Stroke::calcSize()
{
	if (this->pointCount == 0)
	{
		Element::x = 0;
		Element::y = 0;

		// The size of the rectangle, not the size of the pen!
		Element::width = 0;
		Element::height = 0;
	}

	double minX = DBL_MAX;
	double maxX = DBL_MIN;
	double minY = DBL_MAX;
	double maxY = DBL_MIN;

	bool hasPressure = points[0].z != Point::NO_PRESSURE;
	double halfThick = this->width / 2.0;  //  accommodate for pen width

	for (int i = 0; i < this->pointCount; i++)
	{
		if (hasPressure) halfThick = points[i].z / 2.0;

		minX = std::min(minX, points[i].x - halfThick);
		minY = std::min(minY, points[i].y - halfThick);

		maxX = std::max(maxX, points[i].x + halfThick);
		maxY = std::max(maxY, points[i].y + halfThick);
	}

	Element::x = minX - 2;
	Element::y = minY - 2;
	Element::width = maxX - minX + 4;
	Element::height = maxY - minY + 4;
}

EraseableStroke* Stroke::getEraseable()
{
	return this->eraseable;
}

void Stroke::setEraseable(EraseableStroke* eraseable)
{
	this->eraseable = eraseable;
}

void Stroke::debugPrint()
{
	g_message("%s", FC(FORMAT_STR("Stroke {1} / hasPressure() = {2}") % (uint64_t) this % this->hasPressure()));

	for (int i = 0; i < this->pointCount; i++)
	{
		Point p = this->points[i];
		g_message("%lf / %lf", p.x, p.y);
	}

	g_message("\n");
}
