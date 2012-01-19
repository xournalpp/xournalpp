#include "TextEditor.h"
#include <gtk/gtkimcontextsimple.h>
#include "XournalView.h"
#include "PageView.h"
#include "../view/TextView.h"
#include <gdk/gdkkeysyms.h>
#include "../control/Control.h"
#include <string.h>

#include "TextEditorWidget.h"
#include "../undo/TextUndoAction.h"
#include "../undo/ColorUndoAction.h"
#include "../view/DocumentView.h"
#include "Cursor.h"

// TODO LOW PRIO: implement drag & drop

TextEditor::TextEditor(PageView * gui, GtkWidget * widget, Text * text, bool ownText) {
	XOJ_INIT_TYPE(TextEditor);

	this->gui = gui;
	this->widget = widget;
	this->text = text;
	this->text->setInEditing(true);
	this->ownText = ownText;
	this->cursorVisible = false;
	this->blinkTimeout = 0;
	this->cursorOverwrite = false;
	this->needImReset = false;
	this->textWidget = gtk_xoj_int_txt_new(this);
	this->layout = NULL;
	this->virtualCursor = 0;
	this->markPosX = 0;
	this->markPosY = 0;
	this->markPosExtendSelection = false;
	this->markPosQueue = false;
	this->mouseDown = 0;
	this->undoActions = NULL;
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

	GtkSettings * settings = gtk_widget_get_settings(this->widget);
	g_object_get(settings, "gtk-cursor-blink-time", &this->cursorBlinkTime, NULL);
	g_object_get(settings, "gtk-cursor-blink-timeout", &this->cursorBlinkTimeout, NULL);

	this->imContext = gtk_im_context_simple_new();
	gtk_im_context_focus_in(this->imContext);

	g_signal_connect (this->imContext, "commit",
			G_CALLBACK (iMCommitCallback), this);
	g_signal_connect (this->imContext, "preedit-changed",
			G_CALLBACK (iMPreeditChangedCallback), this);
	g_signal_connect (this->imContext, "retrieve-surrounding",
			G_CALLBACK (iMRetrieveSurroundingCallback), this);
	g_signal_connect (this->imContext, "delete-surrounding",
			G_CALLBACK (imDeleteSurroundingCallback), this);

	blinkCallback(this);
}

TextEditor::~TextEditor() {
	XOJ_CHECK_TYPE(TextEditor);

	this->text->setInEditing(false);
	this->widget = NULL;

	Control * control = gui->getXournal()->getControl();
	control->setCopyPasteEnabled(false);

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
			for (GList * l = this->undoActions; l != NULL; l = l->next) {
				TextUndoAction * undo = (TextUndoAction *) l->data;
				undo->textEditFinished();
			}
		}
		g_list_free(this->undoActions);
	}

	if (this->ownText) {
		delete this->text;
		this->text = NULL;
	}

	g_object_unref(this->buffer);
	gtk_widget_destroy(this->textWidget);

	if (this->blinkTimeout) {
		g_source_remove(this->blinkTimeout);
	}

	g_object_unref(this->imContext);

	this->text = NULL;

	if (this->layout) {
		g_object_unref(this->layout);
		this->layout = NULL;
	}

	XOJ_RELEASE_TYPE(TextEditor);
}

Text * TextEditor::getText() {
	XOJ_CHECK_TYPE(TextEditor);

	GtkTextIter start, end;

	gtk_text_buffer_get_bounds(this->buffer, &start, &end);
	char * text = gtk_text_iter_get_text(&start, &end);
	this->text->setText(text);
	g_free(text);

	return this->text;
}

void TextEditor::setText(String text) {
	XOJ_CHECK_TYPE(TextEditor);

	gtk_text_buffer_set_text(this->buffer, text.c_str(), -1);

	GtkTextIter first = { 0 };
	gtk_text_buffer_get_iter_at_offset(this->buffer, &first, 0);
	gtk_text_buffer_place_cursor(this->buffer, &first);
}

UndoAction * TextEditor::setColor(int color) {
	XOJ_CHECK_TYPE(TextEditor);


	int origColor = this->text->getColor();
	this->text->setColor(color);

	repaintEditor();

	// This is a new text, so we don't need to create a undo action
	if(this->ownText) {
		return NULL;
	}

	ColorUndoAction * undo = new ColorUndoAction(gui->getPage(), gui->getPage().getSelectedLayer(), this->gui);
	undo->addStroke(this->text, origColor, color);

	return undo;
}

void TextEditor::setFont(XojFont font) {
	XOJ_CHECK_TYPE(TextEditor);

	this->text->setFont(font);
	TextView::updatePangoFont(this->layout, this->text);
	this->repaintEditor();
}

void TextEditor::textCopyed() {
	XOJ_CHECK_TYPE(TextEditor);

	this->ownText = false;
}

void TextEditor::iMCommitCallback(GtkIMContext * context, const gchar * str, TextEditor * te) {
	XOJ_CHECK_TYPE_OBJ(te, TextEditor);

	gtk_text_buffer_begin_user_action(te->buffer);
	gboolean had_selection = gtk_text_buffer_get_selection_bounds(te->buffer, NULL, NULL);

	gtk_text_buffer_delete_selection(te->buffer, true, true);

	if (!strcmp(str, "\n")) {
		if (!gtk_text_buffer_insert_interactive_at_cursor(te->buffer, "\n", 1, true)) {
			gtk_widget_error_bell(te->widget);
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
			gtk_widget_error_bell(te->widget);
		}
	}

	gtk_text_buffer_end_user_action(te->buffer);
	te->repaintEditor();
	te->contentsChanged();
}

void TextEditor::iMPreeditChangedCallback(GtkIMContext * context, TextEditor * te) {
	XOJ_CHECK_TYPE_OBJ(te, TextEditor);

	gchar * str;
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
		gtk_widget_error_bell(te->widget);
		goto out;
	}

	te->preeditString = str;
	te->repaintEditor();
	te->contentsChanged();

	out:

	pango_attr_list_unref(attrs);
	g_free(str);
}

bool TextEditor::iMRetrieveSurroundingCallback(GtkIMContext * context, TextEditor * te) {
	XOJ_CHECK_TYPE_OBJ(te, TextEditor);

	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_iter_at_mark(te->buffer, &start, gtk_text_buffer_get_insert(te->buffer));
	end = start;

	gint pos = gtk_text_iter_get_line_index(&start);
	gtk_text_iter_set_line_offset(&start, 0);
	gtk_text_iter_forward_to_line_end(&end);

	gchar * text = gtk_text_iter_get_slice(&start, &end);
	gtk_im_context_set_surrounding(context, text, -1, pos);
	g_free(text);

	te->repaintEditor();
	te->contentsChanged();
	return true;
}

bool TextEditor::imDeleteSurroundingCallback(GtkIMContext * context, gint offset, gint n_chars, TextEditor * te) {
	XOJ_CHECK_TYPE_OBJ(te, TextEditor);

	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_iter_at_mark(te->buffer, &start, gtk_text_buffer_get_insert(te->buffer));
	end = start;

	gtk_text_iter_forward_chars(&start, offset);
	gtk_text_iter_forward_chars(&end, offset + n_chars);

	gtk_text_buffer_delete_interactive(te->buffer, &start, &end, true);

	te->repaintEditor();
	te->contentsChanged();

	return true;
}

bool TextEditor::onKeyPressEvent(GdkEventKey * event) {
	XOJ_CHECK_TYPE(TextEditor);

	if (gtk_bindings_activate_event((GtkObject *) this->textWidget, event)) {
		return true;
	}

	bool retval = false;
	bool obscure = false;

	GtkTextIter iter;
	GtkTextMark * insert = gtk_text_buffer_get_insert(this->buffer);
	gtk_text_buffer_get_iter_at_mark(this->buffer, &iter, insert);
	bool canInsert = gtk_text_iter_can_insert(&iter, true);
	if (gtk_im_context_filter_keypress(this->imContext, event)) {
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
	// Pass through Tab as literal tab, unless Control is held down
	else if ((event->keyval == GDK_Tab || event->keyval == GDK_KP_Tab || event->keyval == GDK_ISO_Left_Tab) && !(event->state & GDK_CONTROL_MASK)) {
		resetImContext();
		iMCommitCallback(NULL, "\t", this);
		obscure = true;
		retval = true;
	} else {
		retval = false;
	}

	if (obscure) {
		Cursor * cursor = gui->getXournal()->getCursor();
		cursor->setInvisible(true);
	}

	return retval;
}

bool TextEditor::onKeyReleaseEvent(GdkEventKey * event) {
	XOJ_CHECK_TYPE(TextEditor);

	GtkTextIter iter;

	GtkTextMark * insert = gtk_text_buffer_get_insert(this->buffer);
	gtk_text_buffer_get_iter_at_mark(this->buffer, &iter, insert);
	if (gtk_text_iter_can_insert(&iter, true) && gtk_im_context_filter_keypress(this->imContext, event)) {
		this->needImReset = true;
		return true;
	}
	return false;
}

void TextEditor::toggleOverwrite() {
	XOJ_CHECK_TYPE(TextEditor);

	this->cursorOverwrite = !this->cursorOverwrite;
	repaintCursor();
}

void TextEditor::selectAll() {
	XOJ_CHECK_TYPE(TextEditor);

	GtkTextIter start_iter, end_iter;

	gtk_text_buffer_get_bounds(buffer, &start_iter, &end_iter);
	gtk_text_buffer_select_range(buffer, &start_iter, &end_iter);

	this->repaintEditor();
}

void TextEditor::moveCursor(GtkMovementStep step, int count, bool extendSelection) {
	XOJ_CHECK_TYPE(TextEditor);


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
	moveCursor(&newplace, extendSelection);

	if (updateVirtualCursor) {
		calcVirtualCursor();
	}

	if (gtk_text_iter_equal(&insert, &newplace)) {
		gtk_widget_error_bell(this->widget);
	}

	this->cursorVisible = false;
	if (this->blinkTimeout) {
		g_source_remove(this->blinkTimeout);
	}
	blinkCallback(this);
}

void TextEditor::findPos(GtkTextIter * iter, double xPos, double yPos) {
	XOJ_CHECK_TYPE(TextEditor);

	if (!this->layout) {
		return;
	}

	int index = 0;
	if (!pango_layout_xy_to_index(this->layout, xPos * PANGO_SCALE, yPos * PANGO_SCALE, &index, NULL)) {
		index++;
	}

	gtk_text_iter_set_offset(iter, getCharOffset(index));
}

void TextEditor::contentsChanged(bool forceCreateUndoAction) {
	XOJ_CHECK_TYPE(TextEditor);

	String currentText = getText()->getText();

	if (forceCreateUndoAction || ABS(lastText.length()-currentText.length()) > 100) {
		if (!lastText.isEmpty() && this->undoActions && !(((TextUndoAction*) this->undoActions->data)->getUndoText() == currentText)) {
			TextUndoAction * undo = new TextUndoAction(gui->getPage(), gui->getPage().getSelectedLayer(), this->text, lastText, gui, this);
			UndoRedoHandler * handler = gui->getXournal()->getControl()->getUndoRedoHandler();
			handler->addUndoAction(undo);
			this->undoActions = g_list_append(this->undoActions, undo);
		}
		lastText = currentText;
	}
}

UndoAction * TextEditor::getFirstUndoAction() {
	XOJ_CHECK_TYPE(TextEditor);

	if (this->undoActions) {
		return (UndoAction *) this->undoActions->data;
	}
	return NULL;
}

void TextEditor::markPos(double x, double y, bool extendSelection) {
	XOJ_CHECK_TYPE(TextEditor);

	if (this->layout == NULL) {
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
	repaintCursor();
}

void TextEditor::mousePressed(double x, double y) {
	XOJ_CHECK_TYPE(TextEditor);

	this->mouseDown = true;
	markPos(x, y, false);
}

void TextEditor::mouseMoved(double x, double y) {
	XOJ_CHECK_TYPE(TextEditor);

	if (this->mouseDown) {
		markPos(x, y, true);
	}
}

void TextEditor::mouseReleased() {
	XOJ_CHECK_TYPE(TextEditor);

	this->mouseDown = false;
}

void TextEditor::jumpALine(GtkTextIter * textIter, int count) {
	XOJ_CHECK_TYPE(TextEditor);

	int cursorLine = gtk_text_iter_get_line(textIter);

	if (cursorLine + count < 0) {
		return;
	}

	PangoLayoutLine * line = pango_layout_get_line(this->layout, cursorLine + count);
	if (line == NULL) {
		return;
	}

	int index = 0;
	pango_layout_line_x_to_index(line, this->virtualCursor * PANGO_SCALE, &index, NULL);

	index = getCharOffset(index);

	gtk_text_iter_set_offset(textIter, index);
}

void TextEditor::calcVirtualCursor() {
	XOJ_CHECK_TYPE(TextEditor);

	this->virtualCursor = 0;
	GtkTextIter cursorIter = { 0 };
	GtkTextMark * cursor = gtk_text_buffer_get_insert(this->buffer);
	gtk_text_buffer_get_iter_at_mark(this->buffer, &cursorIter, cursor);

	int offset = getByteOffset(gtk_text_iter_get_offset(&cursorIter));

	PangoRectangle rect = { 0 };
	pango_layout_index_to_pos(this->layout, offset, &rect);
	this->virtualCursor = ((double) rect.x) / PANGO_SCALE;
}

void TextEditor::moveCursor(const GtkTextIter * newLocation, gboolean extendSelection) {
	XOJ_CHECK_TYPE(TextEditor);

	Control * control = gui->getXournal()->getControl();

	if (extendSelection) {
		gtk_text_buffer_move_mark_by_name(this->buffer, "insert", newLocation);
		control->setCopyPasteEnabled(true);
	} else {
		gtk_text_buffer_place_cursor(this->buffer, newLocation);
		control->setCopyPasteEnabled(false);
	}

	this->repaintEditor();
}

static gboolean whitespace(gunichar ch, gpointer user_data) {
	return (ch == ' ' || ch == '\t');
}

static gboolean not_whitespace(gunichar ch, gpointer user_data) {
	return !whitespace(ch, user_data);
}

static gboolean find_whitepace_region(const GtkTextIter * center, GtkTextIter * start, GtkTextIter * end) {
	*start = *center;
	*end = *center;

	if (gtk_text_iter_backward_find_char(start, not_whitespace, NULL, NULL))
		gtk_text_iter_forward_char(start); /* we want the first whitespace... */
	if (whitespace(gtk_text_iter_get_char(end), NULL))
		gtk_text_iter_forward_find_char(end, not_whitespace, NULL, NULL);

	return !gtk_text_iter_equal(start, end);
}

void TextEditor::deleteFromCursor(GtkDeleteType type, int count) {
	XOJ_CHECK_TYPE(TextEditor);

	GtkTextIter insert;
	gboolean leave_one = false;

	this->resetImContext();

	if (type == GTK_DELETE_CHARS) {
		// Char delete deletes the selection, if one exists
		if (gtk_text_buffer_delete_selection(this->buffer, true, true)) {
			this->contentsChanged(true);
			this->repaintEditor();
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
			gtk_widget_error_bell(this->widget);
		}

		gtk_text_buffer_end_user_action(this->buffer);
	} else {
		gtk_widget_error_bell(this->widget);
	}

	this->contentsChanged();
	this->repaintEditor();
}

void TextEditor::backspace() {
	XOJ_CHECK_TYPE(TextEditor);

	GtkTextIter insert;

	resetImContext();

	// Backspace deletes the selection, if one exists
	if (gtk_text_buffer_delete_selection(this->buffer, true, true)) {
		this->repaintEditor();
		this->contentsChanged();
		return;
	}

	gtk_text_buffer_get_iter_at_mark(this->buffer, &insert, gtk_text_buffer_get_insert(this->buffer));

	if (gtk_text_buffer_backspace(this->buffer, &insert, true, true)) {
		this->repaintEditor();
		this->contentsChanged();
	} else {
		gtk_widget_error_bell(this->widget);
	}
}

String TextEditor::getSelection() {
	XOJ_CHECK_TYPE(TextEditor);

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
	XOJ_CHECK_TYPE(TextEditor);

	GtkClipboard * clipboard = gtk_widget_get_clipboard(this->widget, GDK_SELECTION_PRIMARY);
	gtk_text_buffer_copy_clipboard(this->buffer, clipboard);
}

void TextEditor::cutToClipboard() {
	XOJ_CHECK_TYPE(TextEditor);

	GtkClipboard * clipboard = gtk_widget_get_clipboard(this->widget, GDK_SELECTION_PRIMARY);
	gtk_text_buffer_cut_clipboard(this->buffer, clipboard, true);

	this->repaintEditor();
	this->contentsChanged(true);
}

void TextEditor::pasteFromClipboard() {
	XOJ_CHECK_TYPE(TextEditor);

	GtkClipboard * clipboard = gtk_widget_get_clipboard(this->widget, GDK_SELECTION_PRIMARY);
	gtk_text_buffer_paste_clipboard(this->buffer, clipboard, NULL, true);

	this->repaintEditor();
	this->contentsChanged(true);
}

void TextEditor::resetImContext() {
	XOJ_CHECK_TYPE(TextEditor);

	if (this->needImReset) {
		this->needImReset = false;
		gtk_im_context_reset(this->imContext);
	}
}

void TextEditor::repaintCursor() {
	XOJ_CHECK_TYPE(TextEditor);

	repaintEditor();
}

#define CURSOR_ON_MULTIPLIER 2
#define CURSOR_OFF_MULTIPLIER 1
#define CURSOR_PEND_MULTIPLIER 3
#define CURSOR_DIVIDER 3

/*
 * Blink!
 */
gint TextEditor::blinkCallback(TextEditor * te) {
	XOJ_CHECK_TYPE_OBJ(te, TextEditor);

	if (te->cursorVisible) {
		te->blinkTimeout = gdk_threads_add_timeout(te->cursorBlinkTime * CURSOR_OFF_MULTIPLIER / CURSOR_DIVIDER, (GSourceFunc) blinkCallback, te);
	} else {
		te->blinkTimeout = gdk_threads_add_timeout(te->cursorBlinkTime * CURSOR_ON_MULTIPLIER / CURSOR_DIVIDER, (GSourceFunc) blinkCallback, te);
	}

	te->cursorVisible = !te->cursorVisible;

	gdk_threads_enter();
	te->repaintCursor();
	gdk_threads_leave();

	// Remove ourselves
	return false;
}

void TextEditor::repaintEditor() {
	XOJ_CHECK_TYPE(TextEditor);

	//	double x = this->text->getX();
	//	double y = this->text->getY();
	//	double width = this->text->getElementWidth() + 10;
	//	double heigth = this->text->getElementHeight() + 10;
	// TODO OPTIMIZE redraw if the filed is getting smaller (e.g. by pressing <ENTER>)
	//       there is a repaint problem
	//this->gui->repaintRect(x, y, width, heigth);
	this->gui->repaintPage();
}

/**
 * Calculate the UTF-8 Char offset into a byte offset.
 */
int TextEditor::getByteOffset(int charOffset) {
	const char * text = pango_layout_get_text(this->layout);
	return g_utf8_offset_to_pointer(text, charOffset) - text;
}

/**
 * Calculate the UTF-8 Char byte offset into a char offset.
 */
int TextEditor::getCharOffset(int byteOffset) {
	const char * text = pango_layout_get_text(this->layout);

	return g_utf8_pointer_to_offset(text, text + byteOffset);
}

void TextEditor::drawCursor(cairo_t * cr, double x, double y, double height, double zoom) {
	XOJ_CHECK_TYPE(TextEditor);

	double cw = 2 / zoom;
	double dX = 0;
	if (this->cursorOverwrite) {
		dX = -cw / 2;
		cw *= 2;
	}

	// TODO LOW PRIO: with newer cairo use: CAIRO_OPERATOR_DIFFERENCE

	// Not draw cursor if a move is pending
	if (!this->markPosQueue) {
		if (this->cursorVisible) {
			cairo_rectangle(cr, x + dX, y, cw, height);
			cairo_fill(cr);
		}
	}
	DocumentView::applyColor(cr, this->text);
}

void TextEditor::paint(cairo_t * cr, GdkRectangle * repaintRect, double zoom) {
	XOJ_CHECK_TYPE(TextEditor);

	GdkColor selectionColor = this->gui->getSelectionColor();

	cairo_save(cr);

	DocumentView::applyColor(cr, this->text);

	GtkTextIter cursorIter = { 0 };
	GtkTextMark * cursor = gtk_text_buffer_get_insert(this->buffer);
	gtk_text_buffer_get_iter_at_mark(this->buffer, &cursorIter, cursor);

	double x0 = this->text->getX();
	double y0 = this->text->getY();
	cairo_translate(cr, x0, y0);

	if (this->layout == NULL) {
		this->layout = TextView::initPango(cr, this->text);
	}

	if (!this->preeditString.isEmpty()) {
		String text = this->text->getText();
		int pos = gtk_text_iter_get_offset(&cursorIter);
		String txt = text.substring(0, pos);
		txt += preeditString;
		txt += text.substring(pos);

		PangoAttribute * attrib = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
		PangoAttrList * list = pango_layout_get_attributes(this->layout);

		attrib->start_index = pos;
		attrib->end_index = pos + preeditString.size();

		if (list == NULL) {
			list = pango_attr_list_new();
			pango_layout_set_attributes(this->layout, list);
		}
		pango_attr_list_insert(list, attrib);

		pango_layout_set_text(this->layout, txt.c_str(), txt.size());
	} else {
		String txt = this->text->getText();
		pango_layout_set_text(this->layout, txt.c_str(), txt.size());
	}

	GtkTextIter start;
	GtkTextIter end;
	bool hasSelection = gtk_text_buffer_get_selection_bounds(this->buffer, &start, &end);

	if (hasSelection) {
		PangoAttribute * attrib = pango_attr_background_new(selectionColor.red, selectionColor.green, selectionColor.blue);
		PangoAttrList * list = pango_layout_get_attributes(this->layout);

		attrib->start_index = getByteOffset(gtk_text_iter_get_offset(&start));
		attrib->end_index = getByteOffset(gtk_text_iter_get_offset(&end));

		if (list == NULL) {
			list = pango_attr_list_new();
			pango_layout_set_attributes(this->layout, list);
		}
		pango_attr_list_insert(list, attrib);
	} else {
		// remove all attributes
		PangoAttrList * list = pango_attr_list_new();
		pango_layout_set_attributes(this->layout, list);
	}

	pango_cairo_show_layout(cr, this->layout);
	int w = 0;
	int h = 0;
	pango_layout_get_size(this->layout, &w, &h);
	double width = ((double) w) / PANGO_SCALE;
	double height = ((double) h) / PANGO_SCALE;

	int offset = gtk_text_iter_get_offset(&cursorIter);
	PangoRectangle rect = { 0 };
	int pangoOffset = getByteOffset(offset) + preeditString.size();
	pango_layout_index_to_pos(this->layout, pangoOffset, &rect);
	double cX = ((double) rect.x) / PANGO_SCALE;
	double cY = ((double) rect.y) / PANGO_SCALE;
	double cHeight = ((double) rect.height) / PANGO_SCALE;

	drawCursor(cr, cX, cY, cHeight, zoom);

	cairo_restore(cr);

	// set the line always the same size on display
	cairo_set_line_width(cr, 1 / zoom);
	cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue / 65536.0);

	cairo_rectangle(cr, x0 - 5 / zoom, y0 - 5 / zoom, width + 10 / zoom, height + 10 / zoom);
	cairo_stroke(cr);

	this->text->setWidth(width);
	this->text->setHeight(height);

	if (this->markPosQueue) {
		this->markPosQueue = false;
		markPos(this->markPosX, this->markPosY, this->markPosExtendSelection);
	}
}

