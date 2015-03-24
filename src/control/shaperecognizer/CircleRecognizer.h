/*
 * Xournal++
 *
 * Part of the Xournal shape recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __CIRCLE_RECOGNIZER_H__
#define __CIRCLE_RECOGNIZER_H__

class Stroke;
class Inertia;

class CircleRecognizer
{
private:
	CircleRecognizer();
	virtual ~CircleRecognizer();

public:
	static Stroke* recognize(Stroke* s);

private:
	static Stroke* makeCircleShape(Stroke* originalStroke, Inertia& inertia);
	static double scoreCircle(Stroke* s, Inertia& inertia);

};

#endif /* __CIRCLE_RECOGNIZER_H__ */
