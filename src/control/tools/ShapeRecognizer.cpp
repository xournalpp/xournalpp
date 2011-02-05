#include "ShapeRecognizer.h"
#include <math.h>
#include <string.h>
#include "../../model/Stroke.h"

#define TRACELINE() printf("trace:: %i\n", __LINE__);

class Inertia {
public:
	/* compute normalized quantities */
	inline double centerX() {
		return this->sx / this->mass;
	}

	inline double centerY() {
		return this->sy / this->mass;
	}

	inline double xx() {
		if (this->mass <= 0.0) {
			return 0.0;
		}
		return (this->sxx - this->sx * this->sx / this->mass) / this->mass;
	}

	inline double xy() {
		if (this->mass <= 0.0) {
			return 0.0;
		}
		return (this->sxy - this->sx * this->sy / this->mass) / this->mass;
	}

	inline double yy() {
		if (this->mass <= 0.0) {
			return 0.0;
		}
		return (this->syy - this->sy * this->sy / this->mass) / this->mass;
	}

	inline double rad() {
		double ixx = this->xx();
		double iyy = this->yy();
		if (ixx + iyy <= 0.0) {
			return 0.0;
		}
		return sqrt(ixx + iyy);
	}

	inline double det() {
		double ixx = this->xx();
		double iyy = this->yy();
		double ixy = this->xy();
		if (this->mass <= 0.0) {
			return 0.0;
		}

		if (ixx + iyy <= 0.0) {
			return 0.0;
		}

		return 4 * (ixx * iyy - ixy * ixy) / (ixx + iyy) / (ixx + iyy);
	}

	double getMass() {
		return mass;
	}

	/* compute mass and moments of a stroke */
	void increase(Point p1, Point p2, int coef) {
		double dm = coef * hypot(p2.x - p1.x, p2.y - p1.y);
		this->mass += dm;
		this->sx += dm * p1.x;
		this->sy += dm * p1.y;
		this->sxx += dm * p1.x * p1.x;
		this->syy += dm * p1.y * p1.y;
		this->sxy += dm * p1.x * p1.y;
	}

	void calc(const Point * pt, int start, int end) {
		this->mass = this->sx = this->sy = this->sxx = this->sxy = this->syy = 0.;
		for (int i = start; i < end - 1; i++) {
			this->increase(pt[i], pt[i + 1], 1);
		}
	}

private:
	double mass;
	double sx;
	double sy;
	double sxx;
	double sxy;
	double syy;
};

// TODO:recognizer does not always work correct, circle is OK, but lines are usually to long

ShapeRecognizer::ShapeRecognizer() {
	resetRecognizer();
	this->stroke = NULL;
	queueLength = 0;
}

ShapeRecognizer::~ShapeRecognizer() {
	resetRecognizer();
}

void ShapeRecognizer::resetRecognizer() {
	queueLength = 0;
}

/*
 * find the geometry of a recognized segment
 */
void ShapeRecognizer::getSegmentGeometry(const Point * pt, int start, int end, Inertia *s, RecoSegment *r) {
	r->xcenter = s->centerX();
	r->ycenter = s->centerY();
	double a = s->xx();
	double b = s->xy();
	double c = s->yy();
	/* max angle for inertia quadratic form solves: tan(2t) = 2b/(a-c) */
	r->angle = atan2(2 * b, a - c) / 2;
	r->radius = sqrt(3 * (a + c));

	double lmin = 0;
	double lmax = 0;
	for (int i = start; i <= end; i++) {
		double l = (pt[i].x - r->xcenter) * cos(r->angle) + (pt[i].y - r->ycenter) * sin(r->angle);
		if (l < lmin) {
			lmin = l;
		}

		if (l > lmax) {
			lmax = l;
		}
	}
	r->x1 = r->xcenter + lmin * cos(r->angle);
	r->y1 = r->ycenter + lmin * sin(r->angle);
	r->x2 = r->xcenter + lmax * cos(r->angle);
	r->y2 = r->ycenter + lmax * sin(r->angle);
}

Point ShapeRecognizer::calcEdgeIsect(RecoSegment *r1, RecoSegment *r2) {
	double t;
	t = (r2->xcenter - r1->xcenter) * sin(r2->angle) - (r2->ycenter - r1->ycenter) * cos(r2->angle);
	t /= sin(r2->angle - r1->angle);
	double x = r1->xcenter + t * cos(r1->angle);
	double y = r1->ycenter + t * sin(r1->angle);

	return Point(x, y);
}

/*
 *  test if segments form standard shapes
 */
Stroke * ShapeRecognizer::tryRectangle() {
	RecoSegment * rs = NULL;
	RecoSegment * r1 = NULL;
	RecoSegment * r2 = NULL;

	// first, we need whole strokes to combine to 4 segments...
	if (queueLength < 4) {
		TRACELINE();
		return NULL;
	}

	rs = queue + queueLength - 4;
	if (rs->startpt != 0) {
		TRACELINE();
		return NULL;
	}

	// check edges make angles ~= Pi/2 and vertices roughly match
	double avgAngle = 0.;
	for (int i = 0; i <= 3; i++) {
		r1 = rs + i;
		r2 = rs + (i + 1) % 4;
		if (fabs(fabs(r1->angle - r2->angle) - M_PI / 2) > RECTANGLE_ANGLE_TOLERANCE)
			return FALSE;
		avgAngle += r1->angle;
		if (r2->angle > r1->angle) {
			avgAngle += (i + 1) * M_PI / 2;
		} else {
			avgAngle -= (i + 1) * M_PI / 2;
		}

		// test if r1 points away from r2 rather than towards it
		r1->reversed = ((r1->x2 - r1->x1) * (r2->xcenter - r1->xcenter) + (r1->y2 - r1->y1) * (r2->ycenter
				- r1->ycenter)) < 0;
	}
	for (int i = 0; i <= 3; i++) {
		r1 = rs + i;
		r2 = rs + (i + 1) % 4;
		double dist = hypot((r1->reversed ? r1->x1 : r1->x2) - (r2->reversed ? r2->x2 : r2->x1), (r1->reversed ? r1->y1
				: r1->y2) - (r2->reversed ? r2->y2 : r2->y1));
		if (dist > RECTANGLE_LINEAR_TOLERANCE * (r1->radius + r2->radius)) {
			return NULL;
		}
	}

	// make a rectangle of the correct size and slope
	avgAngle = avgAngle / 4;
	if (fabs(avgAngle) < SLANT_TOLERANCE) {
		avgAngle = 0.;
	}

	if (fabs(avgAngle) > M_PI / 2 - SLANT_TOLERANCE) {
		avgAngle = M_PI / 2;
	}

	Stroke * s = new Stroke();
	s->setWidth(this->stroke->getWidth());
	s->setToolType(this->stroke->getToolType());
	s->setColor(this->stroke->getColor());

	for (int i = 0; i <= 3; i++) {
		rs[i].angle = avgAngle + i * M_PI / 2;
	}

	for (int i = 0; i <= 3; i++) {
		Point p = calcEdgeIsect(rs + i, rs + (i + 1) % 4);
		s->addPoint(p);
	}

	s->addPoint(s->getPoint(0));

	return s;
}

Stroke * ShapeRecognizer::tryArrow() {
	double alpha[3], dist, tmp, delta;
	double x1, y1, x2, y2, angle;
	bool rev[3];

	// first, we need whole strokes to combine to nsides segments...
	if (queueLength < 3) {
		TRACELINE();
		return NULL;
	}

	RecoSegment * rs = queue + queueLength - 3;
	if (rs->startpt != 0) {
		TRACELINE();
		return NULL;
	}

	// check arrow head not too big, and orient main segment
	for (int i = 1; i <= 2; i++) {
		if (rs[i].radius > ARROW_MAXSIZE * rs[0].radius) {
			TRACELINE();
			return NULL;
		}

		rev[i] = (hypot(rs[i].xcenter - rs->x1, rs[i].ycenter - rs->y1) < hypot(rs[i].xcenter - rs->x2, rs[i].ycenter
				- rs->y2));
	}

	if (rev[1] != rev[2]) {
		TRACELINE();
		return NULL;
	}

	if (rev[1]) {
		x1 = rs->x2;
		y1 = rs->y2;
		x2 = rs->x1;
		y2 = rs->y1;
		angle = rs->angle + M_PI;
	} else {
		x1 = rs->x1;
		y1 = rs->y1;
		x2 = rs->x2;
		y2 = rs->y2;
		angle = rs->angle;
	}

	// check arrow head not too big, and angles roughly ok
	for (int i = 1; i <= 2; i++) {
		rs[i].reversed = FALSE;
		alpha[i] = rs[i].angle - angle;
		while (alpha[i] < -M_PI / 2) {
			alpha[i] += M_PI;
			rs[i].reversed = !rs[i].reversed;
		}
		while (alpha[i] > M_PI / 2) {
			alpha[i] -= M_PI;
			rs[i].reversed = !rs[i].reversed;
		}
#ifdef RECOGNIZER_DEBUG
		printf("DEBUG: arrow: alpha[%d] = %.1f degrees\n", i, alpha[i] * 180 / M_PI);
#endif
		if (fabs(alpha[i]) < ARROW_ANGLE_MIN || fabs(alpha[i]) > ARROW_ANGLE_MAX) {
			TRACELINE();
			return NULL;
		}
	}

	// check arrow head segments are roughly symmetric
	if (alpha[1] * alpha[2] > 0 || fabs(alpha[1] + alpha[2]) > ARROW_ASYMMETRY_MAX_ANGLE) {
		TRACELINE();
		return NULL;
	}

	if (rs[1].radius / rs[2].radius > 1 + ARROW_ASYMMETRY_MAX_LINEAR) {
		TRACELINE();
		return NULL;
	}

	if (rs[2].radius / rs[1].radius > 1 + ARROW_ASYMMETRY_MAX_LINEAR) {
		TRACELINE();
		return NULL;
	}

	// check vertices roughly match
	Point pt = calcEdgeIsect(rs + 1, rs + 2);
	for (int j = 1; j <= 2; j++) {
		dist = hypot(pt.x - (rs[j].reversed ? rs[j].x1 : rs[j].x2), pt.y - (rs[j].reversed ? rs[j].y1 : rs[j].y2));
#ifdef RECOGNIZER_DEBUG
		printf("DEBUG: linear tolerance: tip[%d] = %.2f\n", j, dist / rs[j].radius);
#endif
		if (dist > ARROW_TIP_LINEAR_TOLERANCE * rs[j].radius) {
			TRACELINE();
			return NULL;
		}
	}

	dist = (pt.x - x2) * sin(angle) - (pt.y - y2) * cos(angle);
	dist /= rs[1].radius + rs[2].radius;

#ifdef RECOGNIZER_DEBUG
	printf("DEBUG: sideways gap tolerance = %.2f\n", dist);
#endif

	if (fabs(dist) > ARROW_SIDEWAYS_GAP_TOLERANCE) {
		TRACELINE();
		return NULL;
	}

	dist = (pt.x - x2) * cos(angle) + (pt.y - y2) * sin(angle);
	dist /= rs[1].radius + rs[2].radius;

#ifdef RECOGNIZER_DEBUG
	printf("DEBUG: main linear gap = %.2f\n", dist);
#endif

	if (dist < ARROW_MAIN_LINEAR_GAP_MIN || dist > ARROW_MAIN_LINEAR_GAP_MAX) {
		TRACELINE();
		return NULL;
	}

	// make an arrow of the correct size and slope
	if (fabs(rs->angle) < SLANT_TOLERANCE) { // nearly horizontal
		angle = angle - rs->angle;
		y1 = y2 = rs->ycenter;
	}

	if (rs->angle > M_PI / 2 - SLANT_TOLERANCE) { // nearly vertical
		angle = angle - (rs->angle - M_PI / 2);
		x1 = x2 = rs->xcenter;
	}

	if (rs->angle < -M_PI / 2 + SLANT_TOLERANCE) { // nearly vertical
		angle = angle - (rs->angle + M_PI / 2);
		x1 = x2 = rs->xcenter;
	}

	delta = fabs(alpha[1] - alpha[2]) / 2;
	dist = (hypot(rs[1].x1 - rs[1].x2, rs[1].y1 - rs[1].y2) + hypot(rs[2].x1 - rs[2].x2, rs[2].y1 - rs[2].y2)) / 2;

	Stroke * s = new Stroke();
	s->setWidth(this->stroke->getWidth());
	s->setToolType(this->stroke->getToolType());
	s->setColor(this->stroke->getColor());

	s->addPoint(Point(x1, y1));
	s->addPoint(Point(x2, y2));

	s->addPoint(Point(x2 - dist * cos(angle + delta), y2 - dist * sin(angle + delta)));
	s->addPoint(Point(x2, y2));

	s->addPoint(Point(x2 - dist * cos(angle - delta), y2 - dist * sin(angle - delta)));

	TRACELINE();
	return s;
}

/*
 *  test if we have a circle; inertia has been precomputed by caller
 */
double ShapeRecognizer::scoreCircle(Inertia * s) {
	if (s->getMass() == 0.0) {
		return 0;
	}

	double sum = 0.0;
	double x0 = s->centerX();
	double y0 = s->centerY();
	double r0 = s->rad();

	ArrayIterator<Point> it = this->stroke->pointIterator();

	if (!it.hasNext()) {
		return 0;
	}

	Point p1 = it.next();

	while (it.hasNext()) {
		Point p2 = it.next();

		double dm = hypot(p2.x - p1.x, p2.y - p1.y);
		double deltar = hypot(p1.x - x0, p1.y - y0) - r0;
		sum += dm * fabs(deltar);

		p1 = p2;
	}

	return sum / (s->getMass() * r0);
}

/*
 * replace strokes by various shapes
 */
Stroke * ShapeRecognizer::makeCircleShape(double x0, double y0, double r) {
	int npts = (int) (2 * r);
	if (npts < 12) {
		npts = 12; // min. number of points
	}

	Stroke * s = new Stroke();
	s->setWidth(this->stroke->getWidth());
	s->setToolType(this->stroke->getToolType());
	s->setColor(this->stroke->getColor());

	for (int i = 0; i <= npts; i++) {
		double x = x0 + r * cos((2 * M_PI * i) / npts);
		double y = y0 + r * sin((2 * M_PI * i) / npts);
		s->addPoint(Point(x, y));
	}

	return s;
}

/*
 * check if something is a polygonal line with at most nsides sides
 */
int ShapeRecognizer::findPolygonal(const Point * pt, int start, int end, int nsides, int *breaks, Inertia *ss) {
	Inertia s;
	Inertia s1;
	Inertia s2;
	int k, i1, i2, n1, n2;
	double det1, det2;

	if (end == start) {
		return 0; // no way
	}

	if (nsides <= 0) {
		return 0;
	}

	if (end - start < 5) {
		nsides = 1; // too small for a polygon
	}

	// look for a linear piece that's big enough
	for (k = 0; k < nsides; k++) {
		i1 = start + (k * (end - start)) / nsides;
		i2 = start + ((k + 1) * (end - start)) / nsides;
		s.calc(pt, i1, i2);
		if (s.det() < LINE_MAX_DET) {
			break;
		}
	}
	if (k == nsides) {
		return 0; // failed!
	}

	// grow the linear piece we found
	while (true) {
		if (i1 > start) {
			s1 = s;
			s1.increase(pt[i1 - 1], pt[i1], 1);
			det1 = s1.det();
		} else {
			det1 = 1.0;
		}

		if (i2 < end) {
			s2 = s;
			s2.increase(pt[i2], pt[i2 + 1], 1);
			det2 = s2.det();
		} else {
			det2 = 1.0;
		}

		if (det1 < det2 && det1 < LINE_MAX_DET) {
			i1--;
			s = s1;
		} else if (det2 < det1 && det2 < LINE_MAX_DET) {
			i2++;
			s = s2;
		} else {
			break;
		}
	}

	if (i1 > start) {
		n1 = findPolygonal(pt, start, i1, (i2 == end) ? (nsides - 1) : (nsides - 2), breaks, ss);
		if (n1 == 0) {
			return 0; // it doesn't work
		}
	} else {
		n1 = 0;
	}

	breaks[n1] = i1;
	breaks[n1 + 1] = i2;
	ss[n1] = s;

	if (i2 < end) {
		n2 = findPolygonal(pt, i2, end, nsides - n1 - 1, breaks + n1 + 1, ss + n1 + 1);
		if (n2 == 0) {
			return 0;
		}
	} else {
		n2 = 0;
	}

	return n1 + n2 + 1;
}

/**
 * Improve on the polygon found by find_polygonal()
 */
void ShapeRecognizer::optimizePolygonal(const Point *pt, int nsides, int * breaks, Inertia * ss) {
	double cost;

	for (int i = 1; i < nsides; i++) {
		// optimize break between sides i and i+1
		cost = ss[i - 1].det() * ss[i - 1].det() + ss[i].det() * ss[i].det();
		Inertia s1 = ss[i - 1];
		Inertia s2 = ss[i];
		bool improved = false;
		while (breaks[i] > breaks[i - 1] + 1) {
			// try moving the break to the left
			s1.increase(pt[breaks[i] - 1], pt[breaks[i] - 2], -1);
			s2.increase(pt[breaks[i] - 1], pt[breaks[i] - 2], 1);
			double newcost = s1.det() * s1.det() + s2.det() * s2.det();

			if (newcost >= cost) {
				break;
			}

			improved = true;
			cost = newcost;
			breaks[i]--;
			ss[i - 1] = s1;
			ss[i] = s2;
		}

		if (improved) {
			continue;
		}

		s1 = ss[i - 1];
		s2 = ss[i];
		while (breaks[i] < breaks[i + 1] - 1) {
			// try moving the break to the right
			s1.increase(pt[breaks[i]], pt[breaks[i] + 1], 1);
			s2.increase(pt[breaks[i]], pt[breaks[i] + 1], -1);

			double newcost = s1.det() * s1.det() + s2.det() * s2.det();
			if (newcost >= cost) {
				break;
			}

			cost = newcost;
			breaks[i]++;
			ss[i - 1] = s1;
			ss[i] = s2;
		}
	}
}

Stroke * ShapeRecognizer::tryClosedPolygon(int nsides) {
	RecoSegment *r1, *r2;
	double dist;

	// first, we need whole strokes to combine to nsides segments...
	if (queueLength < nsides) {
		return NULL;
	}

	RecoSegment *rs = queue + queueLength - nsides;
	if (rs->startpt != 0) {
		return NULL;
	}

	// check vertices roughly match
	for (int i = 0; i < nsides; i++) {
		r1 = rs + i;
		r2 = rs + (i + 1) % nsides;
		// test if r1 points away from r2 rather than towards it
		Point pt = calcEdgeIsect(r1, r2);
		r1->reversed = (hypot(pt.x - r1->x1, pt.y - r1->y1) < hypot(pt.x - r1->x2, pt.y - r1->y2));
	}

	for (int i = 0; i < nsides; i++) {
		r1 = rs + i;
		r2 = rs + (i + 1) % nsides;
		Point pt = calcEdgeIsect(r1, r2);
		dist = hypot((r1->reversed ? r1->x1 : r1->x2) - pt.x, (r1->reversed ? r1->y1 : r1->y2) - pt.y) + hypot(
				(r2->reversed ? r2->x2 : r2->x1) - pt.x, (r2->reversed ? r2->y2 : r2->y1) - pt.y);
		if (dist > POLYGON_LINEAR_TOLERANCE * (r1->radius + r2->radius)) {
			return NULL;
		}
	}

	Stroke * s = new Stroke();
	s->setWidth(this->stroke->getWidth());
	s->setToolType(this->stroke->getToolType());
	s->setColor(this->stroke->getColor());

	for (int i = 0; i < nsides; i++) {
		Point p = calcEdgeIsect(rs + i, rs + (i + 1) % nsides);
		s->addPoint(p);
	}

	s->addPoint(s->getPoint(0));

	return s;
}

/*
 * the main pattern recognition function
 */
Stroke * ShapeRecognizer::recognizePatterns(Stroke * stroke) {
	Inertia s;

	resetRecognizer();
	this->stroke = stroke;

	s.calc(stroke->getPoints(), 0, stroke->getPointCount());

#ifdef RECOGNIZER_DEBUG
	printf("DEBUG: Mass=%.0f, Center=(%.1f,%.1f), I=(%.0f,%.0f, %.0f), "
		"Rad=%.2f, Det=%.4f \n", s.getMass(), s.centerX(), s.centerY(), s.xx(), s.yy(), s.xy(), s.rad(), s.det());
#endif
	Inertia ss[4];
	int brk[5] = { 0 };

	// first see if it's a polygon
	int n = findPolygonal(stroke->getPoints(), 0, stroke->getPointCount(), MAX_POLYGON_SIDES, brk, ss);
	if (n > 0) {
		optimizePolygonal(stroke->getPoints(), n, brk, ss);
#ifdef RECOGNIZER_DEBUG
		printf("\n");
		printf("DEBUG: Polygon, %d edges:\n", n);
		for (int i = 0; i < n; i++) {
			printf("DEBUG:      %d-%d (M=%.0f, det=%.4f)\n", brk[i], brk[i + 1], ss[i].getMass(), ss[i].det());
		}
		printf("\n");
#endif
		/* update recognizer segment queue (most recent at end) */
		while (n + queueLength > MAX_POLYGON_SIDES) {
			// remove oldest polygonal stroke
			int i = 1;
			while (i < queueLength && queue[i].startpt != 0)
				i++;
			queueLength -= i;
			g_memmove(queue, queue+i, queueLength * sizeof(RecoSegment));
		}

#ifdef RECOGNIZER_DEBUG
		printf("DEBUG: Queue now has %d + %d edges\n", queueLength, n);
#endif

		RecoSegment *rs = queue + queueLength;
		queueLength += n;

		for (int i = 0; i < n; i++) {
			rs[i].stroke = stroke;
			rs[i].startpt = brk[i];
			rs[i].endpt = brk[i + 1];
			getSegmentGeometry(stroke->getPoints(), brk[i], brk[i + 1], ss + i, rs + i);
		}

		Stroke * tmp = NULL;

		if ((tmp = tryRectangle()) != NULL) {
			resetRecognizer();
			return tmp;
		}

		if ((tmp = tryArrow()) != NULL) {
			resetRecognizer();
			return tmp;
		}

		if ((tmp = tryClosedPolygon(3)) != NULL) {
			resetRecognizer();
			return tmp;
		}

		if ((tmp = tryClosedPolygon(4)) != NULL) {
			resetRecognizer();
			return tmp;
		}

		if (n == 1) { // current stroke is a line
			if (fabs(rs->angle) < SLANT_TOLERANCE) { // nearly horizontal
				rs->angle = 0.;
				rs->y1 = rs->y2 = rs->ycenter;
			}
			if (fabs(rs->angle) > M_PI / 2 - SLANT_TOLERANCE) { // nearly vertical
				rs->angle = (rs->angle > 0) ? (M_PI / 2) : (-M_PI / 2);
				rs->x1 = rs->x2 = rs->xcenter;
			}

			Stroke * s = new Stroke();
			s->setWidth(this->stroke->getWidth());
			s->setToolType(this->stroke->getToolType());
			s->setColor(this->stroke->getColor());

			s->addPoint(Point(rs->x1, rs->y1));
			s->addPoint(Point(rs->x2, rs->y2));

			return s;
		}
	}

	// not a polygon: maybe a circle ?
	resetRecognizer();

	if (s.det() > CIRCLE_MIN_DET) {
		double score = scoreCircle(&s);
#ifdef RECOGNIZER_DEBUG
		printf("DEBUG: Circle score: %.2f\n", score);
#endif
		if (score < CIRCLE_MAX_SCORE) {
			return makeCircleShape(s.centerX(), s.centerY(), s.rad());
		}
	}

	return NULL;
}
