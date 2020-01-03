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

class LoadHandler;

class LoadHandlerHelper {
private:
    LoadHandlerHelper();
    ~LoadHandlerHelper();

public:
    static int parseBackgroundColor(LoadHandler* loadHandler);
    static bool parseColor(const char* text, int& color, LoadHandler* loadHandler);

    static const char* getAttrib(const char* name, bool optional, LoadHandler* loadHandler);
    static double getAttribDouble(const char* name, LoadHandler* loadHandler);
    static int getAttribInt(const char* name, LoadHandler* loadHandler);
    static bool getAttribInt(const char* name, bool optional, LoadHandler* loadHandler, int& rValue);
    static size_t getAttribSizeT(const char* name, LoadHandler* loadHandler);
    static bool getAttribSizeT(const char* name, bool optional, LoadHandler* loadHandler, size_t& rValue);
};
