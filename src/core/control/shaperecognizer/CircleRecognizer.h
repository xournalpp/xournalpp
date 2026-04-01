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

#include <memory>
class Stroke;
class Inertia;

class CircleRecognizer {
private:
    CircleRecognizer();
    virtual ~CircleRecognizer();

public:
    static auto recognize(Stroke* s) -> std::unique_ptr<Stroke>;

private:
    static auto makeCircleShape(Stroke* originalStroke, Inertia& inertia) -> std::unique_ptr<Stroke>;
    static auto scoreCircle(Stroke* s, Inertia& inertia) -> double;
};
