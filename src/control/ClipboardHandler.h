/*
 * Xournal++
 *
 * The main Control
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __CLIPBOARDHANDLER_H__
#define __CLIPBOARDHANDLER_H__

#include <gtk/gtk.h>
#include "../model/String.h"
#include "EditSelection.h"

class ClipboardListener {
public:
	virtual void clipboardCutCopyEnabled(bool enabled) = 0;
	virtual void clipboardPasteEnabled(bool enabled) = 0;
	virtual void clipboardPasteText(String text) = 0;
	virtual void deleteSelection() = 0;
};

class ClipboardHandler {
public:
	ClipboardHandler(ClipboardListener * listener, GtkWidget * widget);
	virtual ~ClipboardHandler();

	void paste();
	void cut();
	void copy();

	void setSelection(EditSelection * selection);

	void setCopyPasteEnabled(bool enabled);

private:
	static void ownerChangedCallback(GtkClipboard *clip, GdkEvent *event, ClipboardHandler * handler);
	void clipboardUpdated(GdkAtom atom);
	static void receivedClipboardContents(GtkClipboard *clipboard, GtkSelectionData *selectionData,
			ClipboardHandler * handler);

	static void pasteClipboardContents(GtkClipboard *clipboard, GtkSelectionData *selectionData,
			ClipboardHandler * handler);

private:
	ClipboardListener * listener;
	GtkClipboard * clipboard;
	gulong hanlderId;

	EditSelection * selection;

	bool containsText;
};

#endif /* __CLIPBOARDHANDLER_H__ */
