/*
 * Xournal++
 *
 * Internationalization module
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
using std::string;

#include <vector>
using std::vector;

class PlaceholderElement;

/**
 * Placeholder String, used for formatting. Support Placeholder like
 * {1}, {2} etc. Use {{ for {
 */
class PlaceholderString {
public:
	PlaceholderString(string text);
	~PlaceholderString();

	// Placeholder methods
public:
	PlaceholderString& operator%(uint64_t value);
	PlaceholderString& operator%(int value);
	PlaceholderString& operator%(string value);

private:
	string formatPart(string format);
	void process();

	// Process Method
public:
	string str();
	const char* c_str();

private:

	/**
	 * Values for Placeholder
	 */
	vector<PlaceholderElement*> data;

	/**
	 * Input text
	 */
	string text;

	/**
	 * Processed String
	 */
	string processed;
};

std::ostream &operator<<(std::ostream &os, PlaceholderString &ps);

