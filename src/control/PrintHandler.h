/*
 * Xournal++
 *
 * Prints a document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>

class Document;
class Settings;
class SElement;

class PrintHandler
{
public:
	PrintHandler();
	virtual ~PrintHandler();

public:
	void print(Document* doc, int currentPage);

private:
	static void drawPage(GtkPrintOperation* operation, GtkPrintContext* context, int pageNr, PrintHandler* handler);
	static void requestPageSetup(GtkPrintOperation* operation,
								 GtkPrintContext* context, gint pageNr,
								 GtkPageSetup* setup, PrintHandler* handler);

private:
	Document* doc = nullptr;
};
