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

#ifndef __STRING_HPP__
#define __STRING_HPP__
#include <gtk/gtk.h>

class _RefStrInternal;

class String {
public:
	String();
	String(const String & str);
	String(const char * data);
	String(char * data, gboolean freeAutomatically);
	~String();

	String& operator=(const String &str);
	String& operator=(const char * str);

	gboolean operator ==(const String & str) const;
	gboolean operator !=(const String & str) const;
	bool operator <(const String & str) const;
	bool operator >(const String & str) const;

	void operator +=(const String & str);
	void operator +=(const char * str);
	void operator +=(int i);
	void operator +=(double d);

	String replace(String search, String replace) const;

	const gchar * c_str() const;

	//	gchar * reserveBuffer(int buffer);

	int indexOf(String substr, int fromIndex = 0) const;
	int indexOfCaseInsensitiv(String substr, int fromIndex) const;
	int lastIndexOf(String substr) const;
	int lastIndexOf(String substr, int fromIndex) const;

	gboolean contains(const gchar * substr) const;

	gboolean equals(const gchar * other) const;
	gboolean equals(const String & s) const;
	gboolean isEmpty() const;
	gboolean startsWith(const String & s) const;
	gboolean startsWith(const char * s) const;
	gboolean endsWith(const String & s) const;
	gboolean endsWith(const char * s) const;

	gboolean equalsIgnorCase(const String & s);

	int length() const;
	String substring(int start, int length) const;
	String substring(int start) const;
	String trim();

	String toLowerCase() const;
	String toUpperCase() const;
private:
	_RefStrInternal * data;
};

class StringTokenizer {
public:
	StringTokenizer(String s, char token, bool returnToken = false);
	~StringTokenizer();

	const char * next();
private:
	char * str;
	int x;
	int len;
	char token;
	char tokenStr[2];
	bool returnToken;
	bool lastWasToken;
};

#endif /* __STRING_HPP__ */
