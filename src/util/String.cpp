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

#include <String.h>
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
		XOJ_INIT_TYPE(_RefStrInternal);

		this->s = str;
		if (str == NULL) {
			this->size = 0;
			this->len = 0;
		} else {
			this->size = strlen(str);
			this->len = g_utf8_strlen(str, -1);

			const char * end = NULL;
			if (!g_utf8_validate(str, -1, &end)) {
				g_warning("Invalid UTF8 string: %s", str);
			}
		}

		this->nref = 0;
	}

	~_RefStrInternal() {
		XOJ_CHECK_TYPE(_RefStrInternal);

		g_free(this->s);
		this->s = NULL;

		XOJ_RELEASE_TYPE(_RefStrInternal);
	}

	void reference() {
		XOJ_CHECK_TYPE(_RefStrInternal);

		this->nref++;
	}

	void unreference() {
		XOJ_CHECK_TYPE(_RefStrInternal);

		this->nref--;
		if (nref == 0) {
			delete this;
		}
	}

	char * c_str() {
		XOJ_CHECK_TYPE(_RefStrInternal);
		return this->s;
	}

	int getLength() const {
		XOJ_CHECK_TYPE(_RefStrInternal);

		return this->len;
	}

	int getSize() const {
		XOJ_CHECK_TYPE(_RefStrInternal);

		return this->size;
	}

private:
	_RefStrInternal(const _RefStrInternal & str) {
		XOJ_INIT_TYPE(_RefStrInternal);
	}

	_RefStrInternal() {
		XOJ_INIT_TYPE(_RefStrInternal);
	}

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * The reference counter
	 */
	int nref;

	/**
	 * The string
	 */
	char * s;

	/**
	 * The size of the string in bytes
	 */
	int size;

	/**
	 * The length in Unicode chars (may multibyte <= size)
	 */
	int len;
};

/**
 * Creates an empty String
 */
String::String() {
	XOJ_INIT_TYPE(String);

	this->data = new _RefStrInternal(NULL);
	this->data->reference();
}

/**
 * Copies a String
 */
String::String(const String & str) {
	XOJ_INIT_TYPE(String);

	this->data = str.data;
	this->data->reference();
}

/**
 * Copies a const char *
 */
String::String(const char * str) {
	XOJ_INIT_TYPE(String);

	this->data = new _RefStrInternal(g_strdup(str));
	this->data->reference();
}

/**
 * Copies a const char * with a specific length
 */
String::String(const char * data, int len) {
	XOJ_INIT_TYPE(String);

	char * str = (char *) g_malloc(len + 1);
	strncpy(str, data, len);
	str[len] = 0;

	this->data = new _RefStrInternal(str);
	this->data->reference();
}

/**
 * Uses str as internal string if freeAutomatically is true the string will be freed automatically,
 * and NOT COPIED
 */
String::String(char * str, bool freeAutomatically) {
	XOJ_INIT_TYPE(String);

	if (freeAutomatically) {
		this->data = new _RefStrInternal(str);
	} else {
		this->data = new _RefStrInternal(g_strdup(str));
	}
	this->data->reference();
}

String::~String() {
	XOJ_CHECK_TYPE(String);

	this->data->unreference();
	this->data = NULL;

	XOJ_RELEASE_TYPE(String);
}

/**
 * Creates a new String, syntax like sprintf
 */
String String::format(const char * format, ...) {
	va_list args;
	va_start(args, format);
	char * data = g_strdup_vprintf(format, args);

	return String(data, true);
}

/**
 * Searches a substr case insensitive
 */
int String::indexOfCaseInsensitiv(const String substr, int fromIndex) const {
	XOJ_CHECK_TYPE(String);

	return toLowerCase().indexOf(substr.toLowerCase(), fromIndex);
}

int String::indexOf(String substr, int fromIndex) const {
	XOJ_CHECK_TYPE(String);

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
				return g_utf8_pointer_to_offset(source, source + i - sourceOffset);
			}
		}
	}
	return -1;
}

int String::lastIndexOf(const String substr) const {
	XOJ_CHECK_TYPE(String);

	return lastIndexOf(substr, size());
}

int String::lastIndexOf(const String substr, int from) const {
	XOJ_CHECK_TYPE(String);
	int result = -1;

	const char * cStr = c_str();
	const char * cSubstr = substr.c_str();

	if (cStr != NULL && cSubstr != NULL) {
		int compLen = substr.size();

		for (int x = from - compLen; x >= 0; x--) {
			if ((strncmp(cStr + x, cSubstr, compLen)) == 0) {
				result = x;
				break;
			}
		}

	}
	return result;
}

bool String::contains(const char * substr) const {
	XOJ_CHECK_TYPE(String);

	if (c_str() == NULL || substr == NULL) {
		return false;
	}
	return g_strrstr(c_str(), substr) != NULL;
}

bool String::equals(const char * other) const {
	XOJ_CHECK_TYPE(String);

	if (other == c_str()) {
		return true;
	}
	if (other == NULL || c_str() == NULL) {
		return false;
	}
	return strcmp(c_str(), other) == 0;
}

bool String::equals(const String & s) const {
	XOJ_CHECK_TYPE(String);

	return equals(s.c_str());
}

String & String::operator=(const char * str) {
	XOJ_CHECK_TYPE(String);

	this->data->unreference();
	this->data = new _RefStrInternal(g_strdup(str));
	this->data->reference();

	return *this;
}

String& String::operator=(const String & str) {
	XOJ_CHECK_TYPE(String);

	if (this->data) {
		this->data->unreference();
	}
	this->data = str.data;
	if (this->data) {
		this->data->reference();
	}
	return *this;
}

bool String::operator==(const String & str) const {
	XOJ_CHECK_TYPE(String);

	return equals(str.c_str());
}

bool String::operator!=(const String & str) const {
	XOJ_CHECK_TYPE(String);

	return !equals(str.c_str());
}

void String::operator +=(const String & str) {
	XOJ_CHECK_TYPE(String);

	*this += str.c_str();
}

void String::operator +=(int i) {
	XOJ_CHECK_TYPE(String);

	char * tmp = g_strdup_printf("%i", i);
	*this += tmp;
	g_free(tmp);
}

void String::operator +=(double d) {
	XOJ_CHECK_TYPE(String);

	char * tmp = g_strdup_printf("%0.2lf", d);
	*this += tmp;
	g_free(tmp);
}

void String::operator +=(const char * str) {
	XOJ_CHECK_TYPE(String);

	if (str == NULL) {
		return;
	}

	if (c_str() == NULL) {
		this->data->unreference();
		this->data = new _RefStrInternal(g_strdup(str));
		this->data->reference();
	} else {
		char * data = g_strconcat(c_str(), str, NULL);
		this->data->unreference();
		this->data = new _RefStrInternal(data);
		this->data->reference();
	}
}

String String::operator +(const String & str) const {
	String newString = *this;
	newString += str;
	return newString;
}

String String::operator +(const char * str) const {
	String newString = *this;
	newString += str;
	return newString;
}

String String::operator +(int i) const {
	String newString = *this;
	newString += i;
	return newString;
}

String String::operator +(double d) const {
	String newString = *this;
	newString += d;
	return newString;
}

bool String::operator <(const String & str) const {
	XOJ_CHECK_TYPE(String);

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
	XOJ_CHECK_TYPE(String);

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
	XOJ_CHECK_TYPE(String);

	return data->c_str();
}

bool String::isEmpty() const {
	XOJ_CHECK_TYPE(String);

	return this->c_str() == NULL || *c_str() == 0;
}

/**
 * the lengths is the count of chars which this string contains,
 * this is may smaller than the size, if there are multibyte UTF8 chars
 */
int String::length() const {
	XOJ_CHECK_TYPE(String);

	return this->data->getLength();
}

/**
 * The size is the count of bytes which this string contains
 */
int String::size() const {
	XOJ_CHECK_TYPE(String);

	return this->data->getSize();
}

String String::substring(int start) const {
	XOJ_CHECK_TYPE(String);

	if (start < 0) {
		start = length() + start;
		if (start < 0) {
			return NULL;
		}
		return substring(start);
	}

	return substring(start, length() - start);
}

String String::substring(int start, int length) const {
	XOJ_CHECK_TYPE(String);

	if (length < 0) {
		length = this->length() - start + length;
	}

	if (start < 0) {
		start = this->length() + start;
	}

	if (start + length > this->length()) {
		g_critical("substring \"%s\" (%i, %i) out of bounds (to long)", c_str(), start, length);
		length = this->length() - start;
	}
	if (start < 0 || length < 0) {
		g_critical("substring \"%s\" (%i, %i) out of bounds", c_str(), start, length);
		return "";
	}

	const utf8 string = (utf8) c_str();
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

	String substr(substring, true);
	return substr;
}

String String::trim() const {
	XOJ_CHECK_TYPE(String);

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

	const char * orig = c_str();
	char * data = g_strndup(&orig[start], len);

	return String(data, true);
}

bool String::startsWith(const String & s) const {
	XOJ_CHECK_TYPE(String);

	return startsWith(s.c_str());
}

bool String::startsWith(const char * s) const {
	XOJ_CHECK_TYPE(String);

	return g_str_has_prefix(c_str(), s);
}

bool String::endsWith(const String & s) const {
	XOJ_CHECK_TYPE(String);

	return endsWith(s.c_str());
}

bool String::equalsIgnorCase(const String & s) const {
	XOJ_CHECK_TYPE(String);

	if (s.size() != size()) {
		return false;
	}

	return this->toLowerCase().equals(s.toLowerCase());
}

bool String::endsWith(const char * s) const {
	XOJ_CHECK_TYPE(String);

	return g_str_has_suffix(c_str(), s);
}

String String::toLowerCase() const {
	XOJ_CHECK_TYPE(String);

	char * data = g_utf8_strdown(c_str(), size());

	return String(data, true);
}

String String::toUpperCase() const {
	XOJ_CHECK_TYPE(String);

	char * data = g_utf8_strup(c_str(), size());

	return String(data, true);
}

String String::replace(const String search, const String replace) const {
	XOJ_CHECK_TYPE(String);

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
	for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen) {
		patcnt++;
	}

	// allocate memory for the new string
	int const retlen = orilen + patcnt * (replen - patlen);
	char * const returned = (char *) g_malloc(sizeof(char) * (retlen + 1));

	if (returned != NULL) {
		// copy the original string,
		// replacing all the instances of the pattern
		char * retptr = returned;
		for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen) {
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
 */
StringTokenizer::StringTokenizer(const String s, char token, bool returnToken) {
	XOJ_INIT_TYPE(StringTokenizer);

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
	XOJ_CHECK_TYPE(StringTokenizer);

	g_free(this->str);
	this->str = NULL;

	XOJ_RELEASE_TYPE(StringTokenizer);
}

const char * StringTokenizer::next() {
	XOJ_CHECK_TYPE(StringTokenizer);

	if (this->x == -1) {
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

