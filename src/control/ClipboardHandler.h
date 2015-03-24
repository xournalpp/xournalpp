/*
 * Xournal++
 *
 * The main Control
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __CLIPBOARDHANDLER_H__
#define __CLIPBOARDHANDLER_H__

#include <gtk/gtk.h>
#include <StringUtils.h>
#include <XournalType.h>
#include "tools/EditSelection.h"

class ObjectInputStream;

class ClipboardListener
{
public:
	virtual void clipboardCutCopyEnabled(bool enabled) = 0;
	virtual void clipboardPasteEnabled(bool enabled) = 0;
	virtual void clipboardPasteText(string text) = 0;
	virtual void clipboardPasteImage(GdkPixbuf* img) = 0;
	virtual void clipboardPasteXournal(ObjectInputStream& in) = 0;
	virtual void deleteSelection() = 0;
};

class ClipboardHandler
{
public:
	ClipboardHandler(ClipboardListener* listener, GtkWidget* widget);
	virtual ~ClipboardHandler();

public:
	bool paste();
	bool cut();
	bool copy();

	void setSelection(EditSelection* selection);

	void setCopyPasteEnabled(bool enabled);

private:
	static void ownerChangedCallback(GtkClipboard* clip, GdkEvent* event,
									ClipboardHandler* handler);
	void clipboardUpdated(GdkAtom atom);
	static void receivedClipboardContents(GtkClipboard* clipboard,
										GtkSelectionData* selectionData, ClipboardHandler* handler);

	static void pasteClipboardContents(GtkClipboard* clipboard,
									GtkSelectionData* selectionData, ClipboardHandler* handler);
	static void pasteClipboardImage(GtkClipboard* clipboard, GdkPixbuf* pixbuf,
									ClipboardHandler* handler);

private:
	XOJ_TYPE_ATTRIB;

	ClipboardListener* listener;
	GtkClipboard* clipboard;
	gulong hanlderId;

	EditSelection* selection;

	bool containsText;
	bool containsXournal;
	bool containsImage;
};

#endif /* __CLIPBOARDHANDLER_H__ */
