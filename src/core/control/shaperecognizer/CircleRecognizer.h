/*
 * Xournal++
 *
 * Part of the Xournal shape recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class Stroke;
class Inertia;

class CircleRecognizer {
private:
    CircleRecognizer();
    virtual ~CircleRecognizer();

public:
    static Stroke* recognize(Stroke* s);

private:
    static Stroke* makeCircleShape(Stroke* originalStroke, Inertia& inertia);
    static double scoreCircle(Stroke* s, Inertia& inertia);
};
