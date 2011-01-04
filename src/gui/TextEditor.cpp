#include "TextEditor.h"
#include <gtk/gtkimcontextsimple.h>
#include "XournalWidget.h"
#include "PageView.h"
#include "../view/TextView.h"
#include <gdk/gdkkeysyms.h>
#include "../control/Control.h"

#include "TextEditorWidget.h"

class CharContents {
public:
	char c;
	double width;
};

class LineConentents {
public:
	LineConentents() {
		this->charContents = NULL;
	}
	~LineConentents() {
		for (GList * l = this->charContents; l != NULL; l = l->next) {
			CharContents * c = (CharContents *) l->data;
			delete c;
		}
		g_list_free(this->charContents);
		this->charContents = NULL;
	}

	void insertChar(double width, char ch) {
		CharContents * c = new CharContents();
		c->width = width;
		c->c = ch;
		this->charContents = g_list_append(this->charContents, c);
	}

	double y;
	double height;

	GList * charContents;
};

// TODO: enable / disable copy paste buttons

TextEditor::TextEditor(PageView * gui, Text * text, bool ownText) {
	this->gui = gui;
	this->text = text;
	this->text->setInEditing(true);
	this->ownText = ownText;
	this->cursorVisible = false;
	this->blinkTimeout = 0;
	this->cursorOverwrite = false;
	this->needImReset = false;
	this->textWidget = gtk_xoj_int_txt_new(this);
	this->contents = NULL;
	this->virtualCursor = 0;
	this->markPosX = 0;
	this->markPosY = 0;
	this->markPosExtendSelection = false;
	this->markPosQueue = false;
	this->mouseDown = 0;
	this->undoActions = NULL;
	this->fContentsChanged = false;
	this->lastText = text->getText();

	this->buffer = gtk_text_buffer_new(NULL);
	const char * txt = this->text->getText().c_str();
	if (txt == NULL) {
		txt = "";
	}
	gtk_text_buffer_set_text(this->buffer, txt, -1);

	GtkTextIter first = { 0 };
	gtk_text_buffer_get_iter_at_offset(this->buffer, &first, 0);
	gtk_text_buffer_place_cursor(this->buffer, &first);

	GtkSettings *settings = gtk_widget_get_settings(gui->getWidget());
	g_object_get(settings, "gtk-cursor-blink-time", &this->cursorBlinkTime, NULL);
	g_object_get(settings, "gtk-cursor-blink-timeout", &this->cursorBlinkTimeout, NULL);

	this->im_context = gtk_im_context_simple_new();
	gtk_im_context_focus_in(this->im_context);

	g_signal_connect (this->im_context, "commit",
			G_CALLBACK (iMCommitCallback), this);
	g_signal_connect (this->im_context, "preedit-changed",
			G_CALLBACK (iMPreeditChangedCallback), this);
	g_signal_connect (this->im_context, "retrieve-surrounding",
			G_CALLBACK (iMRetrieveSurroundingCallback), this);
	g_signal_connect (this->im_context, "delete-surrounding",
			G_CALLBACK (imDeleteSurroundingCallback), this);

	blinkCallback(this);
}

TextEditor::~TextEditor() {
	this->text->setInEditing(false);

	this->contentsChanged(true);

	if (this->undoActions) {
		if (this->ownText) {
			UndoRedoHandler * handler = gui->getXournal()->getControl()->getUndoRedoHandler();
			for (GList * l = this->undoActions; l != NULL; l = l->next) {
				TextUndoAction * undo = (TextUndoAction *) l->data;
				handler->removeUndoAction(undo);
				delete undo;
			}
		} else {
			UndoRedoHandler * handler = gui->getXournal()->getControl()->getUndoRedoHandler();
			for (GList * l = this->undoActions; l != NULL; l = l->next) {
				TextUndoAction * undo = (TextUndoAction *) l->data;
				undo->textEditFinished();
			}
		}
		g_list_free(this->undoActions);
	}

	if (this->ownText) {
		delete this->text;
	}

	g_object_unref(this->buffer);
	gtk_widget_destroy(this->textWidget);

	if (this->blinkTimeout) {
		g_source_remove(this->blinkTimeout);
	}

	g_object_unref(this->im_context);

	this->text = NULL;
}

Text * TextEditor::getText() {
	GtkTextIter start, end;
	char *text;

	gtk_text_buffer_get_bounds(buffer, &start, &end);
	text = gtk_text_iter_get_text(&start, &end);
	this->text->setText(text);
	g_free(text);

	return this->text;
}

void TextEditor::setText(String text) {
	gtk_text_buffer_set_text(this->buffer, text.c_str(), -1);

	GtkTextIter first = { 0 };
	gtk_text_buffer_get_iter_at_offset(this->buffer, &first, 0);
	gtk_text_buffer_place_cursor(this->buffer, &first);
}

void TextEditor::textCopyed() {
	this->ownText = false;
}

void TextEditor::iMCommitCallback(GtkIMContext *context, const gchar *str, TextEditor * te) {
	gtk_text_buffer_begin_user_action(te->buffer);
	gboolean had_selection = gtk_text_buffer_get_selection_bounds(te->buffer, NULL, NULL);

	gtk_text_buffer_delete_selection(te->buffer, true, true);

	if (!strcmp(str, "\n")) {
		if (!gtk_text_buffer_insert_interactive_at_cursor(te->buffer, "\n", 1, true)) {
			gtk_widget_error_bell(te->gui->getWidget());
		} else {
			te->contentsChanged(true);
		}
	} else {
		if (!had_selection && te->cursorOverwrite) {
			GtkTextIter insert;

			gtk_text_buffer_get_iter_at_mark(te->buffer, &insert, gtk_text_buffer_get_insert(te->buffer));
			if (!gtk_text_iter_ends_line(&insert)) {
				te->deleteFromCursor(GTK_DELETE_CHARS, 1);
			}
		}

		if (!gtk_text_buffer_insert_interactive_at_cursor(te->buffer, str, -1, true)) {
			gtk_widget_error_bell(te->gui->getWidget());
		}
	}

	gtk_text_buffer_end_user_action(te->buffer);
	te->redrawEditor();
	te->contentsChanged();
}

void TextEditor::iMPreeditChangedCallback(GtkIMContext *context, TextEditor * te) {
	gchar *str;
	PangoAttrList *attrs;
	gint cursor_pos;
	GtkTextIter iter;

	gtk_text_buffer_get_iter_at_mark(te->buffer, &iter, gtk_text_buffer_get_insert(te->buffer));

	/* Keypress events are passed to input method even if cursor position is
	 * not editable; so beep here if it's multi-key input sequence, input
	 * method will be reset in key-press-event handler.
	 */
	gtk_im_context_get_preedit_string(context, &str, &attrs, &cursor_pos);

	if (str && str[0] && !gtk_text_iter_can_insert(&iter, true)) {
		gtk_widget_error_bell(te->gui->getWidget());
		goto out;
	}

	te->preeditString = str;
	te->redrawEditor();
	te->contentsChanged();

	out:

	pango_attr_list_unref(attrs);
	g_free(str);
}

bool TextEditor::iMRetrieveSurroundingCallback(GtkIMContext *context, TextEditor * te) {
	GtkTextIter start;
	GtkTextIter end;
	gint pos;
	gchar *text;

	gtk_text_buffer_get_iter_at_mark(te->buffer, &start, gtk_text_buffer_get_insert(te->buffer));
	end = start;

	pos = gtk_text_iter_get_line_index(&start);
	gtk_text_iter_set_line_offset(&start, 0);
	gtk_text_iter_forward_to_line_end(&end);

	text = gtk_text_iter_get_slice(&start, &end);
	gtk_im_context_set_surrounding(context, text, -1, pos);
	g_free(text);

	te->redrawEditor();
	te->contentsChanged();
	return true;
}

bool TextEditor::imDeleteSurroundingCallback(GtkIMContext *context, gint offset, gint n_chars, TextEditor * te) {
	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_iter_at_mark(te->buffer, &start, gtk_text_buffer_get_insert(te->buffer));
	end = start;

	gtk_text_iter_forward_chars(&start, offset);
	gtk_text_iter_forward_chars(&end, offset + n_chars);

	gtk_text_buffer_delete_interactive(te->buffer, &start, &end, true);

	te->redrawEditor();
	te->contentsChanged();

	return true;
}

bool TextEditor::onKeyPressEvent(GdkEventKey *event) {
	if (gtk_bindings_activate_event((GtkObject*) this->textWidget, event)) {
		return true;
	}

	bool retval = false;
	gboolean obscure = false;

	GtkTextIter iter;
	GtkTextMark* insert = gtk_text_buffer_get_insert(this->buffer);
	gtk_text_buffer_get_iter_at_mark(this->buffer, &iter, insert);
	bool canInsert = gtk_text_iter_can_insert(&iter, true);
	if (gtk_im_context_filter_keypress(this->im_context, event)) {
		this->needImReset = true;
		if (!canInsert) {
			this->resetImContext();
		}
		obscure = canInsert;
		retval = true;
	} else if (event->keyval == GDK_Return || event->keyval == GDK_ISO_Enter || event->keyval == GDK_KP_Enter) {
		this->resetImContext();
		iMCommitCallback(NULL, "\n", this);

		obscure = true;
		retval = true;
	}
	/* Pass through Tab as literal tab, unless Control is held down */
	else if ((event->keyval == GDK_Tab || event->keyval == GDK_KP_Tab || event->keyval == GDK_ISO_Left_Tab)
			&& !(event->state & GDK_CONTROL_MASK)) {
		resetImContext();
		iMCommitCallback(NULL, "\t", this);
		obscure = true;
		retval = true;
	} else {
		retval = false;
	}

	if (obscure) {
		Cursor * cursor = gui->getXournal()->getControl()->getCursor();
		cursor->setInvisible(true);
	}

	return retval;
}

bool TextEditor::onKeyReleaseEvent(GdkEventKey *event) {
	GtkTextIter iter;

	GtkTextMark *insert = gtk_text_buffer_get_insert(this->buffer);
	gtk_text_buffer_get_iter_at_mark(this->buffer, &iter, insert);
	if (gtk_text_iter_can_insert(&iter, true) && gtk_im_context_filter_keypress(this->im_context, event)) {
		this->needImReset = true;
		return true;
	}
	return false;
}

void TextEditor::toggleOverwrite() {
	this->cursorOverwrite = !this->cursorOverwrite;
	redrawCursor();
}

void TextEditor::selectAll() {
	GtkTextIter start_iter, end_iter;

	gtk_text_buffer_get_bounds(buffer, &start_iter, &end_iter);
	gtk_text_buffer_select_range(buffer, &start_iter, &end_iter);

	this->redrawEditor();
}

void TextEditor::moveCursor(GtkMovementStep step, int count, bool extend_selection) {
	resetImContext();

	// Not possible, but we have to handle the events, else the page gets scrolled
	//	if (step == GTK_MOVEMENT_PAGES) {
	//		if (!gtk_text_view_scroll_pages(text_view, count, extend_selection))
	//			gtk_widget_error_bell(GTK_WIDGET (text_view));
	//
	//		gtk_text_view_check_cursor_blink(text_view);
	//		gtk_text_view_pend_cursor_blink(text_view);
	//		return;
	//	} else if (step == GTK_MOVEMENT_HORIZONTAL_PAGES) {
	//		if (!gtk_text_view_scroll_hpages(text_view, count, extend_selection))
	//			gtk_widget_error_bell(GTK_WIDGET (text_view));
	//
	//		gtk_text_view_check_cursor_blink(text_view);
	//		gtk_text_view_pend_cursor_blink(text_view);
	//		return;
	//	}

	GtkTextIter insert;
	gtk_text_buffer_get_iter_at_mark(this->buffer, &insert, gtk_text_buffer_get_insert(this->buffer));
	GtkTextIter newplace = insert;

	bool updateVirtualCursor = true;

	switch (step) {
	case GTK_MOVEMENT_LOGICAL_POSITIONS: // not used!?
		gtk_text_iter_forward_visible_cursor_positions(&newplace, count);
		break;
	case GTK_MOVEMENT_VISUAL_POSITIONS:
		if (count < 0) {
			gtk_text_iter_backward_cursor_position(&newplace);
		} else {
			gtk_text_iter_forward_cursor_position(&newplace);
		}
		break;

	case GTK_MOVEMENT_WORDS:
		if (count < 0)
			gtk_text_iter_backward_visible_word_starts(&newplace, -count);
		else if (count > 0) {
			if (!gtk_text_iter_forward_visible_word_ends(&newplace, count)) {
				gtk_text_iter_forward_to_line_end(&newplace);
			}
		}
		break;

	case GTK_MOVEMENT_DISPLAY_LINES:
		updateVirtualCursor = false;
		jumpALine(&newplace, count);
		break;

	case GTK_MOVEMENT_PARAGRAPHS:
		if (count > 0) {
			if (!gtk_text_iter_ends_line(&newplace)) {
				gtk_text_iter_forward_to_line_end(&newplace);
				--count;
			}
			gtk_text_iter_forward_visible_lines(&newplace, count);
			gtk_text_iter_forward_to_line_end(&newplace);
		} else if (count < 0) {
			if (gtk_text_iter_get_line_offset(&newplace) > 0)
				gtk_text_iter_set_line_offset(&newplace, 0);
			gtk_text_iter_forward_visible_lines(&newplace, count);
			gtk_text_iter_set_line_offset(&newplace, 0);
		}
		break;

	case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
	case GTK_MOVEMENT_PARAGRAPH_ENDS:
		if (count > 0) {
			if (!gtk_text_iter_ends_line(&newplace)) {
				gtk_text_iter_forward_to_line_end(&newplace);
			}
		} else if (count < 0) {
			gtk_text_iter_set_line_offset(&newplace, 0);
		}
		break;

	case GTK_MOVEMENT_BUFFER_ENDS:
		if (count > 0) {
			gtk_text_buffer_get_end_iter(this->buffer, &newplace);
		} else if (count < 0) {
			gtk_text_buffer_get_iter_at_offset(this->buffer, &newplace, 0);
		}
		break;

	default:
		break;
	}

	// call moveCursor() even if the cursor hasn't moved, since it cancels the selection
	moveCursor(&newplace, extend_selection);

	if (updateVirtualCursor) {
		calcVirtualCursor();
	}

	if (gtk_text_iter_equal(&insert, &newplace)) {
		gtk_widget_error_bell(gui->getWidget());
	}

	this->cursorVisible = false;
	if (this->blinkTimeout) {
		g_source_remove(this->blinkTimeout);
	}
	blinkCallback(this);
}

void TextEditor::findPos(GtkTextIter * iter, double xPos, double yPos) {
	int line = 0;

	for (GList * l = this->contents; l != NULL; l = l->next, line++) {
		LineConentents * lc = (LineConentents *) l->data;

		if (lc->y > yPos) {
			break;
		}
	}
	gtk_text_iter_set_line(iter, line);

	LineConentents * lc = (LineConentents *) g_list_nth_data(this->contents, line);
	if (!lc) {
		return;
	}

	double x = 0;
	double lastPadding = ABS(xPos - x);
	int id = 0;
	for (GList * l = lc->charContents; l != NULL; l = l->next, id++) {
		CharContents * c = (CharContents *) l->data;
		x += c->width;

		double padding = ABS(xPos - x);
		if (padding > lastPadding) {
			break;
		} else {
			lastPadding = padding;
		}
	}

	gtk_text_iter_set_line_offset(iter, id);
}

void TextEditor::contentsChanged(bool forceCreateUndoAction) {
	this->fContentsChanged = true;

	String currentText = getText()->getText();

	if (forceCreateUndoAction || ABS(lastText.length()-currentText.length()) > 100) {
		if (!lastText.isEmpty() && this->undoActions && !(((TextUndoAction*) this->undoActions->data)->getUndoText()
				== currentText)) {
			TextUndoAction * undo = new TextUndoAction(gui->getPage(), gui->getPage()->getSelectedLayer(), this->text,
					lastText, gui, this);
			UndoRedoHandler * handler = gui->getXournal()->getControl()->getUndoRedoHandler();
			handler->addUndoAction(undo);
			this->undoActions = g_list_append(this->undoActions, undo);
		}
		lastText = currentText;
	}
}

UndoAction * TextEditor::getFirstUndoAction() {
	if (this->undoActions) {
		return (UndoAction *) this->undoActions->data;
	}
	return NULL;
}

void TextEditor::markPos(double x, double y, bool extendSelection) {
	if (this->contents == NULL) {
		this->markPosX = x;
		this->markPosY = y;
		this->markPosExtendSelection = extendSelection;
		this->markPosQueue = true;
		return;
	}
	GtkTextIter iter;
	GtkTextMark* insert = gtk_text_buffer_get_insert(this->buffer);
	gtk_text_buffer_get_iter_at_mark(this->buffer, &iter, insert);
	GtkTextIter newplace = iter;

	findPos(&newplace, x, y);

	// Noting changed
	if (gtk_text_iter_equal(&newplace, &iter)) {
		return;
	}
	moveCursor(&newplace, extendSelection);
	calcVirtualCursor();
	redrawCursor();
}

void TextEditor::mousePressed(double x, double y) {
	this->mouseDown = true;
	markPos(x, y, false);
}

void TextEditor::mouseMoved(double x, double y) {
	if (this->mouseDown) {
		markPos(x, y, true);
	}
}

void TextEditor::mouseReleased() {
	this->mouseDown = false;
}

void TextEditor::jumpALine(GtkTextIter * textIter, int count) {
	int cursorLine = gtk_text_iter_get_line(textIter);

	LineConentents * lc = (LineConentents *) g_list_nth_data(this->contents, cursorLine + count);
	if (!lc) {
		return;
	}

	double x = 0;
	double lastPadding = ABS(this->virtualCursor-x);
	int id = 0;
	for (GList * l = lc->charContents; l != NULL; l = l->next, id++) {
		CharContents * c = (CharContents *) l->data;
		x += c->width;

		double padding = ABS(this->virtualCursor-x);
		if (padding > lastPadding) {
			break;
		} else {
			lastPadding = padding;
		}
	}

	gtk_text_iter_set_line(textIter, cursorLine + count);
	gtk_text_iter_set_line_offset(textIter, id);
}

void TextEditor::calcVirtualCursor() {
	this->virtualCursor = 0;
	GtkTextIter cursorIter = { 0 };
	GtkTextMark * cursor = gtk_text_buffer_get_insert(this->buffer);
	gtk_text_buffer_get_iter_at_mark(this->buffer, &cursorIter, cursor);
	int cursorLine = gtk_text_iter_get_line(&cursorIter);
	int cursorPos = gtk_text_iter_get_line_offset(&cursorIter);

	LineConentents * lc = (LineConentents *) g_list_nth_data(this->contents, cursorLine);
	if (!lc) {
		return;
	}

	int id = 0;
	for (GList * l = lc->charContents; l != NULL && id < cursorPos; l = l->next, id++) {
		CharContents * c = (CharContents *) l->data;
		this->virtualCursor += c->width;
	}
}

void TextEditor::moveCursor(const GtkTextIter *new_location, gboolean extend_selection) {
	if (extend_selection) {
		gtk_text_buffer_move_mark_by_name(this->buffer, "insert", new_location);
	} else {
		gtk_text_buffer_place_cursor(this->buffer, new_location);
	}
}

static gboolean whitespace(gunichar ch, gpointer user_data) {
	return (ch == ' ' || ch == '\t');
}

static gboolean not_whitespace(gunichar ch, gpointer user_data) {
	return !whitespace(ch, user_data);
}

static gboolean find_whitepace_region(const GtkTextIter *center, GtkTextIter *start, GtkTextIter *end) {
	*start = *center;
	*end = *center;

	if (gtk_text_iter_backward_find_char(start, not_whitespace, NULL, NULL))
		gtk_text_iter_forward_char(start); /* we want the first whitespace... */
	if (whitespace(gtk_text_iter_get_char(end), NULL))
		gtk_text_iter_forward_find_char(end, not_whitespace, NULL, NULL);

	return !gtk_text_iter_equal(start, end);
}

void TextEditor::deleteFromCursor(GtkDeleteType type, int count) {
	GtkTextIter insert;
	gboolean leave_one = false;

	this->resetImContext();

	if (type == GTK_DELETE_CHARS) {
		// Char delete deletes the selection, if one exists
		if (gtk_text_buffer_delete_selection(this->buffer, true, true)) {
			this->contentsChanged(true);
			this->redrawEditor();
			return;
		}
	}

	gtk_text_buffer_get_iter_at_mark(this->buffer, &insert, gtk_text_buffer_get_insert(this->buffer));

	GtkTextIter start = insert;
	GtkTextIter end = insert;

	switch (type) {
	case GTK_DELETE_CHARS:
		gtk_text_iter_forward_cursor_positions(&end, count);
		break;

	case GTK_DELETE_WORD_ENDS:
		if (count > 0) {
			gtk_text_iter_forward_word_ends(&end, count);
		} else if (count < 0) {
			gtk_text_iter_backward_word_starts(&start, 0 - count);
		}
		break;

	case GTK_DELETE_WORDS:
		break;

	case GTK_DELETE_DISPLAY_LINE_ENDS:
		break;

	case GTK_DELETE_DISPLAY_LINES:
		break;

	case GTK_DELETE_PARAGRAPH_ENDS:
		if (count > 0) {
			/* If we're already at a newline, we need to
			 * simply delete that newline, instead of
			 * moving to the next one.
			 */
			if (gtk_text_iter_ends_line(&end)) {
				gtk_text_iter_forward_line(&end);
				--count;
			}

			while (count > 0) {
				if (!gtk_text_iter_forward_to_line_end(&end)) {
					break;
				}

				--count;
			}
		} else if (count < 0) {
			if (gtk_text_iter_starts_line(&start)) {
				gtk_text_iter_backward_line(&start);
				if (!gtk_text_iter_ends_line(&end))
					gtk_text_iter_forward_to_line_end(&start);
			} else {
				gtk_text_iter_set_line_offset(&start, 0);
			}
			++count;

			gtk_text_iter_backward_lines(&start, -count);
		}
		break;

	case GTK_DELETE_PARAGRAPHS:
		if (count > 0) {
			gtk_text_iter_set_line_offset(&start, 0);
			gtk_text_iter_forward_to_line_end(&end);

			/* Do the lines beyond the first. */
			while (count > 1) {
				gtk_text_iter_forward_to_line_end(&end);
				--count;
			}
		}

		break;

	case GTK_DELETE_WHITESPACE: {
		find_whitepace_region(&insert, &start, &end);
	}
		break;

	default:
		break;
	}

	if (!gtk_text_iter_equal(&start, &end)) {
		gtk_text_buffer_begin_user_action(this->buffer);

		if (gtk_text_buffer_delete_interactive(this->buffer, &start, &end, true)) {
			if (leave_one) {
				gtk_text_buffer_insert_interactive_at_cursor(this->buffer, " ", 1, true);
			}
		} else {
			gtk_widget_error_bell(gui->getWidget());
		}

		gtk_text_buffer_end_user_action(this->buffer);
	} else {
		gtk_widget_error_bell(gui->getWidget());
	}

	this->contentsChanged();
	this->redrawEditor();
}

void TextEditor::backspace() {
	GtkTextIter insert;

	resetImContext();

	// Backspace deletes the selection, if one exists
	if (gtk_text_buffer_delete_selection(this->buffer, true, true)) {
		this->redrawEditor();
		this->contentsChanged();
		return;
	}

	gtk_text_buffer_get_iter_at_mark(this->buffer, &insert, gtk_text_buffer_get_insert(this->buffer));

	if (gtk_text_buffer_backspace(this->buffer, &insert, true, true)) {
		this->redrawEditor();
		this->contentsChanged();
	} else {
		gtk_widget_error_bell(gui->getWidget());
	}
}

String TextEditor::getSelection() {
	GtkTextIter start, end;
	char *text;
	String s;

	if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
		text = gtk_text_iter_get_text(&start, &end);
		s = text;
		g_free(text);
	}
	return s;
}

void TextEditor::copyToCliboard() {
	GtkClipboard *clipboard = gtk_widget_get_clipboard(this->gui->getWidget(), GDK_SELECTION_PRIMARY);
	gtk_text_buffer_copy_clipboard(this->buffer, clipboard);
}

void TextEditor::cutToClipboard() {
	GtkClipboard *clipboard = gtk_widget_get_clipboard(this->gui->getWidget(), GDK_SELECTION_PRIMARY);
	gtk_text_buffer_cut_clipboard(this->buffer, clipboard, true);

	this->redrawEditor();
	this->contentsChanged(true);
}

void TextEditor::pasteFromClipboard() {
	GtkClipboard *clipboard = gtk_widget_get_clipboard(this->gui->getWidget(), GDK_SELECTION_PRIMARY);
	gtk_text_buffer_paste_clipboard(this->buffer, clipboard, NULL, true);

	this->redrawEditor();
	this->contentsChanged(true);
}

void TextEditor::resetImContext() {
	if (this->needImReset) {
		this->needImReset = false;
		gtk_im_context_reset(this->im_context);
	}
}

void TextEditor::redrawCursor() {
	redrawEditor();
}

#define CURSOR_ON_MULTIPLIER 2
#define CURSOR_OFF_MULTIPLIER 1
#define CURSOR_PEND_MULTIPLIER 3
#define CURSOR_DIVIDER 3

/*
 * Blink!
 */
gint TextEditor::blinkCallback(TextEditor * te) {
	if (te->cursorVisible) {
		te->blinkTimeout = gdk_threads_add_timeout(te->cursorBlinkTime * CURSOR_OFF_MULTIPLIER / CURSOR_DIVIDER,
				(GSourceFunc) blinkCallback, te);
	} else {
		te->blinkTimeout = gdk_threads_add_timeout(te->cursorBlinkTime * CURSOR_ON_MULTIPLIER / CURSOR_DIVIDER,
				(GSourceFunc) blinkCallback, te);
	}

	te->cursorVisible = !te->cursorVisible;
	te->redrawCursor();

	// Remove ourselves
	return false;
}

void TextEditor::redrawEditor() {
	//	double x = this->text->getX();
	//	double y = this->text->getY();
	//	double width = this->text->getElementWidth() + 10;
	//	double heigth = this->text->getElementHeight() + 10;
	//	gui->redrawDocumentRegion(x, y, width, heigth);
	gtk_widget_queue_draw(gui->getWidget());
}

void TextEditor::drawCursor(cairo_t * cr, double & x0, double & x, double & y, cairo_font_extents_t & fe, double zoom) {
	double cw = 2 / zoom;
	double dX = 0;
	if (this->cursorOverwrite) {
		dX = -cw / 2;
		cw *= 2;
	}

	// TODO: with newer cairo use: CAIRO_OPERATOR_DIFFERENCE

	// Not draw cursor if a move is pending
	if (!this->markPosQueue) {
		cairo_rectangle(cr, x0 + x + dX, y - fe.height + fe.descent, cw, fe.height);
		if (this->cursorVisible) {
			cairo_set_source_rgb(cr, 0, 0, 0);
		} else {
			cairo_set_source_rgb(cr, 1, 1, 1);
		}
		cairo_fill(cr);
	}
	DocumentView::applyColor(cr, this->text);

	if (!preeditString.isEmpty()) {
		cairo_text_extents_t ex = { 0 };
		cairo_text_extents(cr, this->preeditString.c_str(), &ex);
		cairo_move_to(cr, x0 + x, y);
		cairo_show_text(cr, this->preeditString.c_str());

		cairo_set_line_width(cr, 1 / zoom);

		cairo_move_to(cr, x0 + x, y + 2);
		x += ex.x_advance;
		cairo_line_to(cr, x0 + x, y + 2);
		cairo_stroke(cr);
	}
}

void TextEditor::paint(cairo_t * cr, GdkEventExpose *event, double zoom) {
	double x0 = this->text->getX();
	double y0 = this->text->getY();
	double y = y0;
	double width = 0;
	GdkColor selectionColor = gui->getSelectionColor();

	DocumentView::applyColor(cr, this->text);
	TextView::initCairo(cr, this->text);

	cairo_text_extents_t extents = { 0 };
	cairo_font_extents_t fe = { 0 };

	// This need to be here, why...? I don't know, the size should be calculated anyway if t->getX() is called...
	this->text->getElementWidth();

	y += 3;
	cairo_font_extents(cr, &fe);

	GtkTextIter iter = { 0 };
	gtk_text_buffer_get_iter_at_offset(this->buffer, &iter, 0);

	gunichar c = 1;

	int line = 0;
	int pos = 0;

	GtkTextIter cursorIter = { 0 };
	GtkTextMark * cursor = gtk_text_buffer_get_insert(this->buffer);
	gtk_text_buffer_get_iter_at_mark(this->buffer, &cursorIter, cursor);
	int cursorLine = gtk_text_iter_get_line(&cursorIter);
	int cursorPos = gtk_text_iter_get_line_offset(&cursorIter);

	GtkTextIter start;
	GtkTextIter end;
	bool hasSelection = gtk_text_buffer_get_selection_bounds(this->buffer, &start, &end);
	bool inSelection = false;

	if (this->fContentsChanged) {
		for (GList * l = this->contents; l != NULL; l = l->next) {
			LineConentents * contents = (LineConentents *) l->data;
			delete contents;
		}

		g_list_free(this->contents);
		this->contents = NULL;
		this->fContentsChanged = false;
	}

	// per line
	do {
		LineConentents * lc = new LineConentents();
		this->contents = g_list_append(this->contents, lc);
		y += fe.height - fe.descent;
		lc->y = y - y0;
		lc->height = fe.height - fe.descent;

		double x = 0;
		pos = 0;

		// per char
		while (!gtk_text_iter_ends_line(&iter) && c) {
			c = gtk_text_iter_get_char(&iter);

			if (hasSelection) {
				if (inSelection && gtk_text_iter_compare(&iter, &end) >= 0) {
					inSelection = false;

					// do not process selection again
					hasSelection = false;
				} else if (!inSelection && gtk_text_iter_compare(&start, &iter) <= 0) {
					inSelection = true;
				}
			}

			gunichar tmp[2] = { 0, 0 };
			tmp[0] = c;
			cairo_text_extents_t ex = { 0 };
			cairo_text_extents(cr, (const char *) tmp, &ex);

			if (inSelection) {
				cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0,
						selectionColor.blue / 65536.0);

				double width = ex.x_advance;
				if (c == '\t') {
					int tab = x / TextView::TAB_INDEX + 1;
					width = tab * TextView::TAB_INDEX - x;
				}

				cairo_rectangle(cr, x + x0 - 0.5, y - fe.height + fe.descent, width + 1, fe.height);
				cairo_fill(cr);

				DocumentView::applyColor(cr, this->text);
			}

			if (cursorLine == line && cursorPos == pos) {
				drawCursor(cr, x0, x, y, fe, zoom);
			}

			if (c == '\t') {
				double lastX = x;
				int tab = x / TextView::TAB_INDEX + 1;
				x = tab * TextView::TAB_INDEX;
				lc->insertChar(x - lastX, '\t');
			} else {
				cairo_move_to(cr, x + x0, y);
				cairo_show_text(cr, (const char *) tmp);

				x += ex.x_advance;
				lc->insertChar(ex.x_advance, (char) c);
			}

			if (!gtk_text_iter_forward_char(&iter)) {
				c = 0;
			}
			pos++;
		}

		if (cursorLine == line && cursorPos == pos) {
			drawCursor(cr, x0, x, y, fe, zoom);
		}

		y += fe.height * 0.25;

		//		printf("\n");

		width = MAX(width, x);

		if (gtk_text_iter_is_end(&iter)) {
			break;
		}

		gtk_text_iter_forward_line(&iter);
		line++;
	} while (c);

	// set the line always the same size on display
	cairo_set_line_width(cr, 1 / zoom);
	cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
			/ 65536.0);

	cairo_rectangle(cr, x0 - 5 / zoom, y0 - 5 / zoom, width + 10 / zoom, y - y0 + 10 / zoom);
	cairo_stroke(cr);

	this->text->setWidth(width);
	this->text->setHeight(y - y0);

	if (this->markPosQueue) {
		this->markPosQueue = false;
		markPos(this->markPosX, this->markPosY, this->markPosExtendSelection);
	}
}

