/*
 * Xournal++
 *
 * Internationalization module
 *
 * @author MarPiRK
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <libintl.h>

#include <string>
using std::string;

/**
 * Placeholder String, used for formatting. Support Placeholder like
 * {1}, {2} etc. Use {{ for {
 */
class PlaceholderString {
public:
	PlaceholderString(const char* text);
	~PlaceholderString();

	// Placeholder methods
public:
	PlaceholderString& operator%(uint64_t value);
	PlaceholderString& operator%(int value);
	PlaceholderString& operator%(string value);

private:
	void process();

	// Process Method
public:
	string str();
	const char* c_str();

private:
	/**
	 * Input text
	 */
	const char* text;
};

std::ostream &operator<<(std::ostream &os, PlaceholderString &ps);

#define _(msg) gettext(msg)
#define C_(context, msg) g_dpgettext (NULL, context "\004" msg, strlen(msg) + 1)

// Formatted Translation
#define _F(msg) PlaceholderString(_(msg))
#define C_F(context, msg) PlaceholderString(C_(context, msg))

// Formatted, not translated text
#define FORMAT_STR(msg) PlaceholderString(msg)


// No translation performed, but in the Translation string
// So translation can be loaded dynamically at other place
// in the code
#define N_(msg) (msg)
#define NC_(context, msg) (msg)

/* Some helper macros */

// boost::locale::format → std::string
#define FS(format) (format).str()
// boost::locale::format → char*
#define FC(format) FS(format).c_str()
