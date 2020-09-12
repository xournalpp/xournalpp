/*
 * Xournal++
 *
 * helper methods to load an .xoj / .xopp document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#pragma once

#include <glib.h>

#include "util/Color.h"

class LoadHandler;

namespace LoadHandlerHelper {
Color parseBackgroundColor(LoadHandler* loadHandler);
bool parseColor(const char* text, Color& color, LoadHandler* loadHandler);

const char* getAttrib(const char* name, bool optional, LoadHandler* loadHandler);
double getAttribDouble(const char* name, LoadHandler* loadHandler);
int getAttribInt(const char* name, LoadHandler* loadHandler);
bool getAttribInt(const char* name, bool optional, LoadHandler* loadHandler, int& rValue);
size_t getAttribSizeT(const char* name, LoadHandler* loadHandler);
bool getAttribSizeT(const char* name, bool optional, LoadHandler* loadHandler, size_t& rValue);
};  // namespace LoadHandlerHelper
