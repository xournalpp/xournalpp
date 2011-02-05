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

#include "String.h"
#include <string.h>
#include <ctype.h>

/**
 * This code is a modified version of what SQLite uses.
 */
#define SKIP_MULTI_BYTE_SEQUENCE(input) {              \
    if( (*(input++)) >= 0xc0 ) {                       \
    while( (*input & 0xc0) == 0x80 ){ input++; }       \
  }                                                    \
}

#define utf8 unsigned char *

class _RefStrInternal {
public:
	_RefStrInternal(char * str) {
		this->s = str;
		if (str == NULL) {
			this->size = 0;
			this->len = 0;
		} else {
			this->size = strlen(str);
			this->len = length((utf8)str);
		}
	}

	~_RefStrInternal() {
		g_free(this->s);
	}

	void reference() {
		nref++;
	}

	void unreference() {
		nref--;
		if (nref == 0) {
			delete this;
		}
	}

	int length(const utf8 string) {
		int len;
		len = 0;
		while (*string) {
			++len;
			SKIP_MULTI_BYTE_SEQUENCE(string);
		}
		return len;
	}

	char * c_str() {
		return s;
	}

	int getLength() const {
		return this->len;
	}

	int getSize() const {
		return this->size;
	}
private:
	_RefStrInternal(const _RefStrInternal & str) {
	}
	_RefStrInternal() {
	}

private:
	int nref;

	char * s;
	int size;
	int len;
};

String::String() {
	this->data = new _RefStrInternal(NULL);
	this->data->reference();
}

String::String(const String & str) {
	*this = str;
}

String::String(const char * str) {
	this->data = new _RefStrInternal(g_strdup(str));
	this->data->reference();
}

String::String(char * str, bool freeAutomatically) {
	if (freeAutomatically) {
		this->data = new _RefStrInternal(str);
	} else {
		this->data = new _RefStrInternal(g_strdup(str));
	}
	this->data->reference();
}

String::~String() {
	data->unreference();
}

int String::indexOfCaseInsensitiv(String substr, int fromIndex) const {
	const char * source = c_str();
	const char * target = substr.c_str();
	if (source == NULL || target == NULL) {
		return -1;
	}
	int sourceCount = size();
	int targetCount = substr.size();

	if (fromIndex >= sourceCount) {
		return (targetCount == 0 ? sourceCount : -1);
	}
	if (fromIndex < 0) {
		fromIndex = 0;
	}
	if (targetCount == 0) {
		return fromIndex;
	}

	int sourceOffset = 0;
	int targetOffset = 0;

	char first = tolower(target[targetOffset]);
	int max = sourceOffset + (sourceCount - targetCount);

	for (int i = sourceOffset + fromIndex; i <= max; i++) {
		/* Look for first character. */
		if (tolower(source[i]) != first) {
			while (++i <= max && tolower(source[i]) != first)
				;
		}

		/* Found first character, now look at the rest of v2 */
		if (i <= max) {
			int j = i + 1;
			int end = j + targetCount - 1;
			for (int k = targetOffset + 1; j < end && tolower(source[j]) == tolower(target[k]); j++, k++)
				;

			if (j == end) {
				/* Found whole string. */
				return i - sourceOffset;
			}
		}
	}
	return -1;
}

int String::indexOf(String substr, int fromIndex) const {
	const char * source = c_str();
	const char * target = substr.c_str();
	if (source == NULL || target == NULL) {
		return -1;
	}
	int sourceCount = size();
	int targetCount = substr.size();

	if (fromIndex >= sourceCount) {
		return (targetCount == 0 ? sourceCount : -1);
	}
	if (fromIndex < 0) {
		fromIndex = 0;
	}
	if (targetCount == 0) {
		return fromIndex;
	}

	int sourceOffset = 0;
	int targetOffset = 0;

	char first = target[targetOffset];
	int max = sourceOffset + (sourceCount - targetCount);

	for (int i = sourceOffset + fromIndex; i <= max; i++) {
		/* Look for first character. */
		if (source[i] != first) {
			while (++i <= max && source[i] != first)
				;
		}

		/* Found first character, now look at the rest of v2 */
		if (i <= max) {
			int j = i + 1;
			int end = j + targetCount - 1;
			for (int k = targetOffset + 1; j < end && source[j] == target[k]; j++, k++)
				;

			if (j == end) {
				/* Found whole string. */
				return i - sourceOffset;
			}
		}
	}
	return -1;
}

int String::lastIndexOf(String substr) const {
	return lastIndexOf(substr, size() - 1);
}

int String::lastIndexOf(String substr, int fromIndex) const {
	const char * source = c_str();
	const char * target = substr.c_str();
	if (source == NULL || target == NULL) {
		return -1;
	}
	int sourceCount = size();
	int targetCount = substr.size();

	if (fromIndex <= 0) {
		return (targetCount == 0 ? 0 : -1);
	}
	if (fromIndex >= sourceCount) {
		fromIndex = sourceCount - 1;
	}
	if (targetCount == 0) {
		return fromIndex;
	}

	char last = target[targetCount - 1];

	for (int i = fromIndex; i >= targetCount; i--) {
		/* Look for first character. */
		if (source[i] != last) {
			while (--i >= targetCount && source[i] != last)
				;
		}

		/* Found last character, now look at the rest of v2 */
		if (i >= 0) {
			int j = i - 1;
			int end = j - targetCount + 1;
			for (int k = targetCount - 2; j >= end && source[j] == target[k]; j--, k--)
				;

			if (j == end) {
				/* Found whole string. */
				return j + 1;
			}
		}
	}
	return -1;
}

bool String::contains(const char * substr) const {
	if (c_str() == NULL || substr == NULL) {
		return false;
	}
	return g_strrstr(c_str(), substr) != NULL;
}

bool String::equals(const char * other) const {
	if (other == c_str()) {
		return true;
	}
	if (other == NULL || c_str() == NULL) {
		return false;
	}
	return strcmp(c_str(), other) == 0;
}

bool String::equals(const String & s) const {
	return equals(s.c_str());
}

String& String::operator=(const char * str) {
	this->data->unreference();
	this->data = new _RefStrInternal(g_strdup(str));
	this->data->reference();
}

String& String::operator=(const String & str) {
	this->data = str.data;
	this->data->reference();
}

bool String::operator==(const String & str) const {
	return equals(str.c_str());
}

bool String::operator!=(const String & str) const {
	return !equals(str.c_str());
}

void String::operator +=(const String & str) {
	*this += str.c_str();
}

void String::operator +=(int i) {
	char * tmp = g_strdup_printf("%i", i);
	*this += tmp;
	g_free(tmp);
}

void String::operator +=(double d) {
	char * tmp = g_strdup_printf("%0.2lf", d);
	*this += tmp;
	g_free(tmp);
}

void String::operator +=(const char * str) {
	char * data = g_strconcat(c_str(), str, NULL);
	this->data->unreference();
	this->data = new _RefStrInternal(data);
	this->data->reference();
}

bool String::operator <(const String & str) const {
	if (c_str() == NULL && str.c_str() == NULL) {
		return false;
	}
	if (c_str() == NULL) {
		return true;
	}
	if (str.c_str() == NULL) {
		return false;
	}

	return strcmp(c_str(), str.c_str()) < 0;
}

bool String::operator >(const String & str) const {
	if (c_str() == NULL && str.c_str() == NULL) {
		return false;
	}
	if (c_str() == NULL) {
		return false;
	}
	if (str.c_str() == NULL) {
		return true;
	}

	return strcmp(c_str(), str.c_str()) > 0;
}

const char * String::c_str() const {
	return data->c_str();
}

bool String::isEmpty() const {
	return this->c_str() == NULL || *c_str() == 0;
}

/**
 * the lengths is the count of chars which this string contains,
 * this is may smaller than the size, if there are multibyte UTF8 chars
 */
int String::length() const {
	return this->data->getLength();
}

/**
 * The size is the count of bytes which this string contains
 */
int String::size() const {
	return this->data->getSize();
}

String String::substring(int start) const {
	if (start < 0) {
		start = length() + start;
		if (start < 0) {
			return NULL;
		}
		return substring(start);
	}

	return substring(start, length() - start);
}

//String oldSubstring(String str, int start, int length) {
//	if (length < 0) {
//		length = str.length() - start + length;
//	}
//
//	if(start < 0) {
//		start = str.length() - start;
//	}
//
//	if (start + length > str.length() || start < 0 || length < 0) {
//		g_critical("substring \"%s\" (%i, %i) out of bounds", str.c_str(), start, length);
//		return "";
//	}
//
//	const char * orig = str.c_str();
//	char * d = (char *) g_malloc(length + 1);
//	strncpy(d, &orig[start], length);
//	d[length] = 0;
//
//	String substr(d, true);
//	return substr;
//}

String String::substring(int start, int length) const {
	if (length < 0) {
		length = this->length() - start + length;
	}

	if (start < 0) {
		start = this->length() - start;
	}

	if (start + length > this->length() || start < 0 || length < 0) {
		g_critical("substring \"%s\" (%i, %i) out of bounds", c_str(), start, length);
		return "";
	}

	const utf8 string = (utf8)c_str();
	int s = start;
	while (*string && s) {
		SKIP_MULTI_BYTE_SEQUENCE(string);
		--s;
	}

	const utf8 str2;
	int l = length;
	for (str2 = string; *str2 && l; l--) {
		SKIP_MULTI_BYTE_SEQUENCE(str2);
	}

	int bytes = (int) (str2 - string);

	char * substring = (char *) g_malloc(bytes + 1);
	char * output = substring;

	for (int i = 0; i < bytes; i++) {
		*output++ = *string++;
	}
	*output = '\0';

//	String old = oldSubstring(*this, start, length);
//
//	const char * error = "";
//
//	if(old != substring) {
//		error = "\nMISSMATCH!";
//	}
//
//	printf("substring(\"%s\", %i, %i) =\n\"%s\"\n\"%s\"%s\n\n", this->c_str(), start, length, substring, old.c_str(),error);

	String substr(substring, true);
	return substr;
}

String String::trim() {
	const char * s = c_str();
	int start = 0;
	int len;

	while (true) {
		char tmp = s[start];
		if (tmp == ' ' || tmp == '\t' || tmp == '\n' || tmp == '\r') {
			start++;
		} else {
			break;
		}
	}

	len = size() - start;

	while (len != 0) {
		char tmp = s[start + len - 1];
		if (tmp == ' ' || tmp == '\t' || tmp == '\n' || tmp == '\r') {
			len--;
		} else {
			break;
		}
	}

	if (start == 0 && len == size()) {
		return *this;
	}

	return substring(start, len);
}

bool String::startsWith(const String & s) const {
	return startsWith(s.c_str());
}

bool String::startsWith(const char * s) const {
	return g_str_has_prefix(c_str(), s);
}

bool String::endsWith(const String & s) const {
	return endsWith(s.c_str());
}

bool String::equalsIgnorCase(const String & s) {
	return this->toLowerCase().equals(s.toLowerCase());
}

bool String::endsWith(const char * s) const {
	return g_str_has_suffix(c_str(), s);
}

String String::toLowerCase() const {
	String s = c_str();

	char * data = s.data->c_str();

	for (int i = 0; i < s.size(); i++) {
		data[i] = tolower(data[i]);
	}

	return s;
}

String String::toUpperCase() const {
	String s = c_str();

	char * data = s.data->c_str();

	for (int i = 0; i < s.size(); i++) {
		data[i] = toupper(data[i]);
	}

	return s;
}

/**
 * TODO: this has some bugs!!
 */
String String::replace(String search, String replace) const {
	char const * const original = c_str();
	char const * const pattern = search.c_str();
	char const * const replacement = replace.c_str();
	int const replen = replace.size();
	int const patlen = search.size();
	int const orilen = size();

	int patcnt = 0;
	const char * oriptr;
	const char * patloc;

	if (original == NULL) {
		return NULL;
	}

	// find how many times the pattern occurs in the original string
	for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen) {
		patcnt++;
	}

	// allocate memory for the new string
	int const retlen = orilen + patcnt * (replen - patlen);
	char * const returned = (char *) g_malloc(sizeof(char) * (retlen + 1));

	if (returned != NULL) {
		// copy the original string,
		// replacing all the instances of the pattern
		char * retptr = returned;
		for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen) {
			int const skplen = patloc - oriptr;
			// copy the section until the occurence of the pattern
			strncpy(retptr, oriptr, skplen);
			retptr += skplen;
			// copy the replacement
			strncpy(retptr, replacement, replen);
			retptr += replen;
		}
		// copy the rest of the string.
		strcpy(retptr, oriptr);
	}

	return String(returned, true);
}

/**
 * String tokenizer
 *
 * THIS CLASS IS NOT UTF8 SAVE
 */
StringTokenizer::StringTokenizer(String s, char token, bool returnToken) {
	this->str = g_strdup(s.c_str());
	this->token = token;
	this->tokenStr[0] = token;
	this->tokenStr[1] = 0;
	this->returnToken = returnToken;
	this->lastWasToken = false;
	this->x = 0;
	this->len = s.size();
}

StringTokenizer::~StringTokenizer() {
	g_free(this->str);
}

const char * StringTokenizer::next() {
	if (x == -1) {
		return NULL;
	}

	if (this->lastWasToken) {
		this->lastWasToken = false;
		return this->tokenStr;
	}

	const char * tmp = this->str + x;

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

