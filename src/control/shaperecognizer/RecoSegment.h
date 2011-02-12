/*
 * Xournal Extended
 *
 * Part of the Xournal shape recognizer
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __RECOSEGMENT_H__
#define __RECOSEGMENT_H__

class Stroke;

class RecoSegment {
public:
	RecoSegment();
	virtual ~RecoSegment();

public:
	Stroke * stroke;
	int startpt;
	int endpt;

	double xcenter;
	double ycenter;
	double angle;
	double radius;

	double x1;
	double y1;
	double x2;
	double y2;

	bool reversed;
};

#endif /* __RECOSEGMENT_H__ */
