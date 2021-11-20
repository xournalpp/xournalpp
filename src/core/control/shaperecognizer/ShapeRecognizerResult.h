/*
 * Xournal++
 *
 * Xournal Shape recognizer result
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>


class Stroke;

class ShapeRecognizer;

class ShapeRecognizerResult {
public:
    ShapeRecognizerResult(Stroke* result);
    ShapeRecognizerResult(Stroke* result, ShapeRecognizer* recognizer);
    virtual ~ShapeRecognizerResult();

public:
    void addSourceStroke(Stroke* s);
    Stroke* getRecognized();
    std::vector<Stroke*>* getSources();

private:
    Stroke* recognized;
    std::vector<Stroke*> source;
};
