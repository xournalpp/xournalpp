/*
 * Xournal++
 *
 * A String class
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#include <StringUtils.h>
#include <ctype.h>
#include <memory>

/**
 * This code is a modified version of what SQLite uses.
 */
#define SKIP_MULTI_BYTE_SEQUENCE(input) {              \
        if( (*(input++)) >= 0xc0 ) {                       \
            while( (*input & 0xc0) == 0x80 ){ input++; }       \
        }                                                    \
    }

/**
 * Creates a new String, syntax like sprintf
 */
String StringUtils::format(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* data = g_strdup_vprintf(format, args);

    return String(data);
}

const char * StringUtils::c_str(const String& str) {
    UErrorCode status = U_ZERO_ERROR;
    int32_t sz = str.length() * 2;
    char* dest = new char[sizeof (*dest) * sz];
    str.extract(dest, sz, NULL, status);
    return dest;
}


/**
 * String tokenizer
 */
StringTokenizer::StringTokenizer(const String s, char token, bool returnToken) {
    XOJ_INIT_TYPE(StringTokenizer);

    this->str = const_cast<char*>(CSTR(s));
    this->token = token;
    this->tokenStr[0] = token;
    this->tokenStr[1] = 0;
    this->returnToken = returnToken;
    this->lastWasToken = false;
    this->x = 0;
    this->len = s.length();
}

StringTokenizer::~StringTokenizer() {
    XOJ_CHECK_TYPE(StringTokenizer);

    g_free(this->str);
    this->str = NULL;

    XOJ_RELEASE_TYPE(StringTokenizer);
}

const char* StringTokenizer::next() {
    XOJ_CHECK_TYPE(StringTokenizer);

    if (this->x == -1) {
        return NULL;
    }

    if (this->lastWasToken) {
        this->lastWasToken = false;
        return this->tokenStr;
    }

    const char* tmp = this->str + x;

    for (; x < this->len; x++) {
        if (this->str[x] == this->token) {
            this->str[x] = 0;
            if (this->returnToken) {
                this->lastWasToken = true;
            }
            x++;
            return tmp;
        }
    }
    this->x = -1;

    return tmp;
}

