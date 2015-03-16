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
#include <string.h>
#include <string>

#include <unicode/unistr.h>
typedef icu::UnicodeString String;
#define CSTR StringUtils::c_str

class StringUtils {
    public:
        static String format(const char* format, ...);
        static const char * c_str(const String& str);
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
