/*
 * Xournal++
 *
 * Reference String which is automatically deleted
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __STRINGUTILS_H__
#define __STRINGUTILS_H__

#include <XournalType.h>

#include <glib.h>
#include <iostream>

#include <unicode/unistr.h>
#include <unicode/msgfmt.h>

using namespace std;
using namespace icu;

typedef UnicodeString String;

#define CSTR StringUtils::c_str
#define CONCAT StringUtils::concat

ostream& operator<<(ostream& ost, const String& str);

class StringUtils {
    public:
        static String format(const char* format, ...);
        static gchar* c_str(const String& str);
       
    //Sorry, I don't know how to move implemendation to StringUtils.cpp
    private:
        static void addToString(String& str) {};
        template<typename T, typename... Args>
        static void addToString(String& str, const T& a_value, Args... a_args)  {
            str += String(a_value);
            addToString(str, a_args...);
        } 
    public:
        template<typename... Args>
        static String* concat(Args... a_args)  {
            String* s = new String();
            addToString(*s, a_args...);
            return s;
        }
};

class StringTokenizer {
    public:
        StringTokenizer(const String s, char token, bool returnToken = false);
        virtual ~StringTokenizer();

        const char* next();

    private:
        XOJ_TYPE_ATTRIB;

        char* str;
        int x;
        int len;
        char token;
        char tokenStr[2];
        bool returnToken;
        bool lastWasToken;
};

#endif /* __STRINGUTILS_H__ */
