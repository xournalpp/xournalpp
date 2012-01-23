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

#ifndef __STRING_H__
#define __STRING_H__

#include <glib.h>
#include <XournalType.h>

class _RefStrInternal;

class String {
public:
	String();
	String(const String & str);
	String(const char * data);
	String(const char * data, int len);
	virtual ~String();

private:
	String(char * data, bool freeAutomatically);

public:
	static String format(const char * format, ...);

public:

	String& operator=(const String & str);
	String& operator=(const char * str);

	bool operator ==(const String & str) const;
	bool operator !=(const String & str) const;
	bool operator <(const String & str) const;
	bool operator >(const String & str) const;

	void operator +=(const String & str);
	void operator +=(const char * str);
	void operator +=(int i);
	void operator +=(double d);

	String operator +(const String & str) const;
	String operator +(const char *) const;
	String operator +(int i) const;
	String operator +(double d) const;

	String replace(const String search, const String replace) const;

	const char * c_str() const;

	int indexOf(const String substr, int fromIndex = 0) const;
	int indexOfCaseInsensitiv(const String substr, int fromIndex = 0) const;
	int lastIndexOf(const String substr) const;
	int lastIndexOf(const String substr, int from) const;

	bool contains(const char * substr) const;

	bool equals(const char * other) const;
	bool equals(const String & s) const;
	bool isEmpty() const;
	bool startsWith(const String & s) const;
	bool startsWith(const char * s) const;
	bool endsWith(const String & s) const;
	bool endsWith(const char * s) const;

	bool equalsIgnorCase(const String & s) const;

	int length() const;
	int size() const;

	/**
	 * like in php, negative start and length are working, and are from end instead of start
	 */
	String substring(int start, int length) const;
	String substring(int start) const;
	String trim() const;

	String toLowerCase() const;
	String toUpperCase() const;

private:
	XOJ_TYPE_ATTRIB;

	_RefStrInternal * data;
};

class StringTokenizer {
public:
	StringTokenizer(const String s, char token, bool returnToken = false);
	virtual ~StringTokenizer();

	const char * next();

private:
	XOJ_TYPE_ATTRIB;

	char * str;
	int x;
	int len;
	char token;
	char tokenStr[2];
	bool returnToken;
	bool lastWasToken;
};

#endif /* __STRING_H__ */
