#include "TextEditor.h"

#include <cassert>
#include <cmath>
#include <cstring>  // for strcmp, size_t
#include <memory>   // for allocator, make_unique, __shared_p...
#include <utility>  // for move

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_B, GDK_KEY_ISO_Enter, GDK_...
#include <glib-object.h>     // for g_object_get, g_object_unref, G_CA...

#include "control/Control.h"       // for Control
#include "model/Font.h"            // for XojFont
#include "model/PageRef.h"         // for PageRef
#include "model/Text.h"            // for Text
#include "model/XojPage.h"         // for XojPage
#include "undo/UndoRedoHandler.h"  // for UndoRedoHandler
#include "util/Range.h"
#include "util/raii/CStringWrapper.h"

#include "PageView.h"          // for XojPageView
#include "TextEditorWidget.h"  // for gtk_xoj_int_txt_new
#include "XournalView.h"       // for XournalView
#include "XournalppCursor.h"   // for XournalppCursor

class UndoAction;

/** GtkTextBuffer helper functions **/
static auto getIteratorAtCursor(GtkTextBuffer* buffer) -> GtkTextIter {
    GtkTextIter cursorIter = {nullptr};
    gtk_text_buffer_get_iter_at_mark(buffer, &cursorIter, gtk_text_buffer_get_insert(buffer));
    return cursorIter;
}

/**
 * @brief Compute the byte offset of an iterator in the GtkTextBuffer
 *
 * NB: This is much faster than relying on g_utf8_offset_to_pointer
 */
static auto getByteOffsetOfIterator(GtkTextIter it) -> int {
    // Bytes from beginning of line to iterator
    int pos = gtk_text_iter_get_line_index(&it);
    gtk_text_iter_set_line_index(&it, 0);
    // Count bytes of previous lines
    while (gtk_text_iter_backward_line(&it)) { pos += gtk_text_iter_get_bytes_in_line(&it); }
    return pos;
}

static auto getByteOffsetOfCursor(GtkTextBuffer* buffer) -> int {
    return getByteOffsetOfIterator(getIteratorAtCursor(buffer));
}

/**
 * @brief Get an iterator at the prescribed byte index.
 *
 * NB: This is much faster than relying on g_utf8_pointer_to_offset for long texts
 */
static auto getIteratorAtByteOffset(GtkTextBuffer* buf, int byteIndex) {
    assert(byteIndex >= 0);
    GtkTextIter it = {nullptr};
    gtk_text_buffer_get_start_iter(buf, &it);

    // Fast forward to the beginning of the line containing our target destination
    for (int linelength = gtk_text_iter_get_bytes_in_line(&it);
         linelength <= byteIndex && gtk_text_iter_forward_line(&it);
         byteIndex -= std::exchange(linelength, gtk_text_iter_get_bytes_in_line(&it))) {}

    if (!gtk_text_iter_is_end(&it)) {
        gtk_text_iter_set_line_index(&it, byteIndex);
    }
    // else { // byteIndex was either past-the-end or pointed to the end }

    return it;
}

static auto cloneToCString(GtkTextBuffer* buf) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    return xoj::util::OwnedCString::assumeOwnership(gtk_text_iter_get_text(&start, &end));
}

/**
 * @brief Clone the buffer's content and insert a string into the clone
 * This makes one less copy operation over using cloneToStdString followed by insert()
 * (The text is copied "only" twice, and not three times)
 */
static auto cloneWithInsertToStdString(GtkTextBuffer* buf, std::string_view insertedStr) -> std::string {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    GtkTextIter insertionPoint = getIteratorAtCursor(buf);

    auto firstHalf = xoj::util::OwnedCString::assumeOwnership(gtk_text_iter_get_text(&start, &insertionPoint));
    auto secondHalf = xoj::util::OwnedCString::assumeOwnership(gtk_text_iter_get_text(&insertionPoint, &end));

    std::string_view str1(firstHalf);
    std::string_view str2(secondHalf);

    std::string res;
    res.reserve(str1.length() + str2.length() + insertedStr.length());
    res += str1;
    res += insertedStr;
    res += str2;

    return res;
}

TextEditor::TextEditor(XojPageView* gui, GtkWidget* widget, Text* text, bool ownText):
        gui(gui),
        xournalWidget(widget),
        text(text),
        textWidget(gtk_xoj_int_txt_new(this), xoj::util::adopt),
        imContext(gtk_im_multicontext_new(), xoj::util::adopt),
        buffer(gtk_text_buffer_new(nullptr), xoj::util::adopt),
        layout(text->createPangoLayout()),
        previousBoundingBox(text->boundingRect()),
        ownText(ownText) {
    this->text->setInEditing(true);

    this->replaceBufferContent(text->getText());

    g_signal_connect(this->buffer.get(), "paste-done", G_CALLBACK(bufferPasteDoneCallback), this);

    {  // Get cursor blinking settings
        GtkSettings* settings = gtk_widget_get_settings(this->xournalWidget);
        g_object_get(settings, "gtk-cursor-blink", &this->cursorBlink, nullptr);
        if (this->cursorBlink) {
            int tmp = 0;
            g_object_get(settings, "gtk-cursor-blink-time", &tmp, nullptr);
            assert(tmp >= 0);
            auto cursorBlinkingPeriod = static_cast<unsigned int>(tmp);
            this->cursorBlinkingTimeOn = cursorBlinkingPeriod * CURSOR_ON_MULTIPLIER / CURSOR_DIVIDER;
            this->cursorBlinkingTimeOff = cursorBlinkingPeriod - this->cursorBlinkingTimeOn;
        }
    }

    gtk_im_context_set_client_window(this->imContext.get(), gtk_widget_get_parent_window(this->xournalWidget));
    gtk_im_context_focus_in(this->imContext.get());

    g_signal_connect(this->imContext.get(), "commit", G_CALLBACK(iMCommitCallback), this);
    g_signal_connect(this->imContext.get(), "preedit-changed", G_CALLBACK(iMPreeditChangedCallback), this);
    g_signal_connect(this->imContext.get(), "retrieve-surrounding", G_CALLBACK(iMRetrieveSurroundingCallback), this);
    g_signal_connect(this->imContext.get(), "delete-surrounding", G_CALLBACK(imDeleteSurroundingCallback), this);


    if (this->cursorBlink) {
        BlinkTimer::callback(this);
    } else {
        this->cursorVisible = true;
    }
}

TextEditor::~TextEditor() {
    gtk_im_context_focus_out(this->imContext.get());

    this->text->setInEditing(false);
    this->xournalWidget = nullptr;

    Control* control = gui->getXournal()->getControl();
    control->setCopyCutEnabled(false);

    this->contentsChanged(true);

    if (this->ownText) {
        delete this->text;
    }
    this->text = nullptr;
}

auto TextEditor::getText() const -> Text* {
    GtkTextIter start, end;

    gtk_text_buffer_get_bounds(this->buffer.get(), &start, &end);
    auto text = xoj::util::OwnedCString::assumeOwnership(gtk_text_iter_get_text(&start, &end));
    this->text->setText(text.get());

    return this->text;
}

void TextEditor::replaceBufferContent(const std::string& text) {
    gtk_text_buffer_set_text(this->buffer.get(), text.c_str(), -1);

    GtkTextIter first = {nullptr};
    gtk_text_buffer_get_iter_at_offset(this->buffer.get(), &first, 0);
    gtk_text_buffer_place_cursor(this->buffer.get(), &first);
}

void TextEditor::setColor(Color color) {
    this->text->setColor(color);
    repaintEditor(false);
}

void TextEditor::setFont(XojFont font) {
    this->text->setFont(font);
    afterFontChange();
}

void TextEditor::afterFontChange() {
    this->text->updatePangoFont(this->layout.get());
    this->computeVirtualCursorPosition();
    this->repaintEditor();
}

void TextEditor::textCopyed() { this->ownText = false; }

void TextEditor::iMCommitCallback(GtkIMContext* context, const gchar* str, TextEditor* te) {
    gtk_text_buffer_begin_user_action(te->buffer.get());

    bool hadSelection = gtk_text_buffer_get_has_selection(te->buffer.get());
    gtk_text_buffer_delete_selection(te->buffer.get(), true, true);

    if (!strcmp(str, "\n")) {
        if (!gtk_text_buffer_insert_interactive_at_cursor(te->buffer.get(), "\n", 1, true)) {
            gtk_widget_error_bell(te->xournalWidget);
        } else {
            te->contentsChanged(true);
        }
    } else {
        if (!hadSelection && te->cursorOverwrite) {
            auto insert = getIteratorAtCursor(te->buffer.get());
            if (!gtk_text_iter_ends_line(&insert)) {
                te->deleteFromCursor(GTK_DELETE_CHARS, 1);
            }
        }

        if (!gtk_text_buffer_insert_interactive_at_cursor(te->buffer.get(), str, -1, true)) {
            gtk_widget_error_bell(te->xournalWidget);
        }
    }

    gtk_text_buffer_end_user_action(te->buffer.get());
    te->contentsChanged();
    te->repaintEditor();
}

void TextEditor::iMPreeditChangedCallback(GtkIMContext* context, TextEditor* te) {
    xoj::util::OwnedCString str;
    gint cursor_pos = 0;
    GtkTextIter iter = getIteratorAtCursor(te->buffer.get());

    {
        PangoAttrList* attrs = nullptr;
        gtk_im_context_get_preedit_string(context, str.contentReplacer(), &attrs, &cursor_pos);
        if (attrs == nullptr) {
            attrs = pango_attr_list_new();
        }
        te->preeditAttrList.reset(attrs, xoj::util::adopt);
    }

    if (str && str[0] && !gtk_text_iter_can_insert(&iter, true)) {
        /*
         * Keypress events are passed to input method even if cursor position is
         * not editable; so beep here if it's multi-key input sequence, input
         * method will be reset in key-press-event handler.
         */
        gtk_widget_error_bell(te->xournalWidget);
        return;
    }

    te->preeditString = std::move(str);
    te->preeditCursor = cursor_pos;
    te->contentsChanged();
    te->repaintEditor();
}

auto TextEditor::iMRetrieveSurroundingCallback(GtkIMContext* context, TextEditor* te) -> bool {
    GtkTextIter start = getIteratorAtCursor(te->buffer.get());
    GtkTextIter end = start;

    gint pos = gtk_text_iter_get_line_index(&start);
    gtk_text_iter_set_line_offset(&start, 0);
    gtk_text_iter_forward_to_line_end(&end);

    auto text = xoj::util::OwnedCString::assumeOwnership(gtk_text_iter_get_slice(&start, &end));
    gtk_im_context_set_surrounding(context, text.get(), -1, pos);

    return true;
}

auto TextEditor::imDeleteSurroundingCallback(GtkIMContext* context, gint offset, gint n_chars, TextEditor* te) -> bool {
    GtkTextIter start = getIteratorAtCursor(te->buffer.get());
    GtkTextIter end = start;

    gtk_text_iter_forward_chars(&start, offset);
    gtk_text_iter_forward_chars(&end, offset + n_chars);

    gtk_text_buffer_delete_interactive(te->buffer.get(), &start, &end, true);

    te->contentsChanged();
    te->repaintEditor();

    return true;
}

auto TextEditor::onKeyPressEvent(GdkEventKey* event) -> bool {
    bool retval = false;
    bool obscure = false;

    GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask();

    GtkTextIter iter = getIteratorAtCursor(this->buffer.get());
    bool canInsert = gtk_text_iter_can_insert(&iter, true);

    // IME needs to handle the input first so the candidate window works correctly
    if (gtk_im_context_filter_keypress(this->imContext.get(), event)) {
        this->needImReset = true;
        if (!canInsert) {
            this->resetImContext();
        }
        obscure = canInsert;
        retval = true;
    } else if (gtk_bindings_activate_event(G_OBJECT(this->textWidget.get()), event)) {
        return true;
    } else if ((event->state & modifiers) == GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_b || event->keyval == GDK_KEY_B) {
            toggleBoldFace();
            return true;
        }
        if (event->keyval == GDK_KEY_plus) {
            increaseFontSize();
            return true;
        }
        if (event->keyval == GDK_KEY_minus) {
            decreaseFontSize();
            return true;
        }
    } else if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_ISO_Enter ||
               event->keyval == GDK_KEY_KP_Enter) {
        this->resetImContext();
        iMCommitCallback(nullptr, "\n", this);

        obscure = true;
        retval = true;
    }
    // Pass through Tab as literal tab, unless Control is held down
    else if ((event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_KP_Tab ||
              event->keyval == GDK_KEY_ISO_Left_Tab) &&
             !(event->state & GDK_CONTROL_MASK)) {
        resetImContext();
        iMCommitCallback(nullptr, "\t", this);
        obscure = true;
        retval = true;
    } else {
        retval = false;
    }

    if (obscure) {
        XournalppCursor* cursor = gui->getXournal()->getCursor();
        cursor->setInvisible(true);
    }

    return retval;
}

auto TextEditor::onKeyReleaseEvent(GdkEventKey* event) -> bool {
    GtkTextIter iter = getIteratorAtCursor(this->buffer.get());

    if (gtk_text_iter_can_insert(&iter, true) && gtk_im_context_filter_keypress(this->imContext.get(), event)) {
        this->needImReset = true;
        return true;
    }
    return false;
}

void TextEditor::toggleOverwrite() {
    this->cursorOverwrite = !this->cursorOverwrite;
    repaintCursor();
}

/**
 * I know it's a bit rough and duplicated
 * Improve that later on...
 */
void TextEditor::decreaseFontSize() {
    XojFont& font = text->getFont();
    if (double size = font.getSize(); size > 1) {
        font.setSize(font.getSize() - 1);
        afterFontChange();
    }
}

void TextEditor::increaseFontSize() {
    XojFont& font = text->getFont();
    font.setSize(font.getSize() + 1);
    afterFontChange();
}

void TextEditor::toggleBoldFace() {
    // get the current/used font
    XojFont& font = text->getFont();
    std::string fontName = font.getName();

    std::size_t found = fontName.find(" Bold");

    // toggle bold
    if (found == std::string::npos) {
        fontName = fontName + " Bold";
    } else {
        fontName = fontName.erase(found, 5);
    }

    font.setName(fontName);
    afterFontChange();
}

void TextEditor::selectAtCursor(TextEditor::SelectType ty) {
    GtkTextIter startPos;
    GtkTextIter endPos;
    gtk_text_buffer_get_selection_bounds(this->buffer.get(), &startPos, &endPos);
    const auto searchFlag = GTK_TEXT_SEARCH_TEXT_ONLY;  // To be used to find double newlines

    switch (ty) {
        case TextEditor::SelectType::WORD: {
            auto currentPos = getIteratorAtCursor(this->buffer.get());
            if (!gtk_text_iter_inside_word(&currentPos)) {
                // Do nothing if cursor is over whitespace
                return;
            }

            if (!gtk_text_iter_starts_word(&currentPos)) {
                gtk_text_iter_backward_word_start(&startPos);
            }
            if (!gtk_text_iter_ends_word(&currentPos)) {
                gtk_text_iter_forward_word_end(&endPos);
            }
            break;
        }
        case TextEditor::SelectType::PARAGRAPH:
            // Note that a GTK "paragraph" is a line, so there's no nice one-liner.
            // We define a paragraph as text separated by double newlines.
            while (!gtk_text_iter_is_start(&startPos)) {
                // There's no GTK function to go to line start, so do it manually.
                while (!gtk_text_iter_starts_line(&startPos)) {
                    if (!gtk_text_iter_backward_word_start(&startPos)) {
                        break;
                    }
                }
                // Check for paragraph start
                GtkTextIter searchPos = startPos;
                gtk_text_iter_backward_chars(&searchPos, 2);
                if (gtk_text_iter_backward_search(&startPos, "\n\n", searchFlag, nullptr, nullptr, &searchPos)) {
                    break;
                }
                gtk_text_iter_backward_line(&startPos);
            }
            while (!gtk_text_iter_ends_line(&endPos)) {
                gtk_text_iter_forward_to_line_end(&endPos);
                // Check for paragraph end
                GtkTextIter searchPos = endPos;
                gtk_text_iter_forward_chars(&searchPos, 2);
                if (gtk_text_iter_forward_search(&endPos, "\n\n", searchFlag, nullptr, nullptr, &searchPos)) {
                    break;
                }
                gtk_text_iter_forward_line(&endPos);
            }
            break;
        case TextEditor::SelectType::ALL:
            gtk_text_buffer_get_bounds(this->buffer.get(), &startPos, &endPos);
            break;
    }

    gtk_text_buffer_select_range(this->buffer.get(), &startPos, &endPos);

    this->repaintEditor(false);
}

void TextEditor::moveCursor(GtkMovementStep step, int count, bool extendSelection) {
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

    GtkTextIter insert = getIteratorAtCursor(this->buffer.get());
    GtkTextIter newplace = insert;

    bool updateVirtualCursor = true;

    switch (step) {
        case GTK_MOVEMENT_LOGICAL_POSITIONS:  // not used!?
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
            if (count < 0) {
                gtk_text_iter_backward_visible_word_starts(&newplace, -count);
            } else if (count > 0) {
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
                if (gtk_text_iter_get_line_offset(&newplace) > 0) {
                    gtk_text_iter_set_line_offset(&newplace, 0);
                }
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
                gtk_text_buffer_get_end_iter(this->buffer.get(), &newplace);
            } else if (count < 0) {
                gtk_text_buffer_get_iter_at_offset(this->buffer.get(), &newplace, 0);
            }
            break;

        default:
            break;
    }

    // call moveCursor() even if the cursor hasn't moved, since it cancels the selection
    moveCursor(&newplace, extendSelection);

    if (updateVirtualCursor) {
        computeVirtualCursorPosition();
    }

    if (gtk_text_iter_equal(&insert, &newplace)) {
        gtk_widget_error_bell(this->xournalWidget);
    }

    if (this->cursorBlink) {
        this->cursorVisible = false;
        BlinkTimer::callback(this);
    } else {
        repaintCursor();
    }
}

void TextEditor::findPos(GtkTextIter* iter, double xPos, double yPos) const {
    if (!this->layout) {
        return;
    }

    int index = 0;
    int trailing = 0;
    pango_layout_xy_to_index(this->layout.get(), static_cast<int>(std::round(xPos * PANGO_SCALE)),
                             static_cast<int>(std::round(yPos * PANGO_SCALE)), &index, &trailing);
    /*
     * trailing is non-zero iff the abscissa is past the middle of the grapheme.
     * In this case, it contains the length of the grapheme in utf8 char count.
     * This way, we put the cursor after the grapheme when clicking past the middle of the grapheme.
     */
    *iter = getIteratorAtByteOffset(this->buffer.get(), index);
    gtk_text_iter_forward_chars(iter, trailing);
}

void TextEditor::contentsChanged(bool forceCreateUndoAction) {
    std::string currentText = getText()->getText();
    // Todo: Reinstate text edition undo stack
    this->computeVirtualCursorPosition();
}

void TextEditor::markPos(double x, double y, bool extendSelection) {
    GtkTextIter newplace = getIteratorAtCursor(this->buffer.get());

    findPos(&newplace, x, y);

    // call moveCursor() even if the cursor hasn't moved, since it cancels the selection
    moveCursor(&newplace, extendSelection);
    computeVirtualCursorPosition();
    repaintCursor();
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

void TextEditor::mouseReleased() { this->mouseDown = false; }

void TextEditor::jumpALine(GtkTextIter* textIter, int count) {
    int cursorLine = gtk_text_iter_get_line(textIter);

    if (cursorLine + count < 0) {
        return;
    }

    PangoLayoutLine* line = pango_layout_get_line_readonly(this->layout.get(), cursorLine + count);
    if (line == nullptr) {
        return;
    }

    int index = 0;
    int trailing = 0;
    pango_layout_line_x_to_index(line, this->virtualCursorAbscissa, &index, &trailing);
    /*
     * trailing is non-zero iff the abscissa is past the middle of the grapheme.
     * In this case, it contains the length of the grapheme in utf8 char count.
     */
    *textIter = getIteratorAtByteOffset(this->buffer.get(), index);
    gtk_text_iter_forward_chars(textIter, trailing);
}

void TextEditor::computeVirtualCursorPosition() {
    int offset = getByteOffsetOfCursor(this->buffer.get());

    PangoRectangle rect = {0};
    pango_layout_index_to_pos(this->layout.get(), offset, &rect);
    this->virtualCursorAbscissa = rect.x;
}

void TextEditor::moveCursor(const GtkTextIter* newLocation, gboolean extendSelection) {
    Control* control = gui->getXournal()->getControl();

    if (extendSelection) {
        gtk_text_buffer_move_mark_by_name(this->buffer.get(), "insert", newLocation);
        control->setCopyCutEnabled(true);
    } else {
        gtk_text_buffer_place_cursor(this->buffer.get(), newLocation);
        control->setCopyCutEnabled(false);
    }

    this->repaintEditor(false);
}

static auto whitespace(gunichar ch, gpointer user_data) -> gboolean { return (ch == ' ' || ch == '\t'); }

static auto not_whitespace(gunichar ch, gpointer user_data) -> gboolean { return !whitespace(ch, user_data); }

static auto find_whitepace_region(const GtkTextIter* center, GtkTextIter* start, GtkTextIter* end) -> gboolean {
    *start = *center;
    *end = *center;

    if (gtk_text_iter_backward_find_char(start, not_whitespace, nullptr, nullptr)) {
        gtk_text_iter_forward_char(start); /* we want the first whitespace... */
    }
    if (whitespace(gtk_text_iter_get_char(end), nullptr)) {
        gtk_text_iter_forward_find_char(end, not_whitespace, nullptr, nullptr);
    }

    return !gtk_text_iter_equal(start, end);
}

void TextEditor::deleteFromCursor(GtkDeleteType type, int count) {

    this->resetImContext();

    if (type == GTK_DELETE_CHARS) {
        // Char delete deletes the selection, if one exists
        if (gtk_text_buffer_delete_selection(this->buffer.get(), true, true)) {
            this->contentsChanged(true);
            this->repaintEditor();
            return;
        }
    }

    GtkTextIter insert = getIteratorAtCursor(this->buffer.get());

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
                    if (!gtk_text_iter_ends_line(&end)) {
                        gtk_text_iter_forward_to_line_end(&start);
                    }
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
        } break;

        default:
            break;
    }

    if (!gtk_text_iter_equal(&start, &end)) {
        gtk_text_buffer_begin_user_action(this->buffer.get());

        if (!gtk_text_buffer_delete_interactive(this->buffer.get(), &start, &end, true)) {
            gtk_widget_error_bell(this->xournalWidget);
        }

        gtk_text_buffer_end_user_action(this->buffer.get());
    } else {
        gtk_widget_error_bell(this->xournalWidget);
    }

    this->contentsChanged();
    this->repaintEditor();
}

void TextEditor::backspace() {

    resetImContext();

    // Backspace deletes the selection, if one exists
    if (gtk_text_buffer_delete_selection(this->buffer.get(), true, true)) {
        this->contentsChanged();
        this->repaintEditor();
        return;
    }

    GtkTextIter insert = getIteratorAtCursor(this->buffer.get());

    if (gtk_text_buffer_backspace(this->buffer.get(), &insert, true, true)) {
        this->contentsChanged();
        this->repaintEditor();
    } else {
        gtk_widget_error_bell(this->xournalWidget);
    }
}

auto TextEditor::getSelection() const -> std::string {
    std::string s;

    if (GtkTextIter start, end; gtk_text_buffer_get_selection_bounds(this->buffer.get(), &start, &end)) {
        auto text = xoj::util::OwnedCString::assumeOwnership(gtk_text_iter_get_text(&start, &end));
        s = text.get();
    }
    return s;
}

void TextEditor::copyToClipboard() const {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget, GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard(this->buffer.get(), clipboard);
}

void TextEditor::cutToClipboard() {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget, GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_cut_clipboard(this->buffer.get(), clipboard, true);

    this->contentsChanged(true);
    this->repaintEditor();
}

void TextEditor::pasteFromClipboard() {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget, GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_paste_clipboard(this->buffer.get(), clipboard, nullptr, true);
}

void TextEditor::bufferPasteDoneCallback(GtkTextBuffer* buffer, GtkClipboard* clipboard, TextEditor* te) {
    te->contentsChanged(true);
    te->repaintEditor();
}

void TextEditor::resetImContext() {
    if (this->needImReset) {
        this->needImReset = false;
        gtk_im_context_reset(this->imContext.get());
    }
}

void TextEditor::repaintCursor() { this->repaintEditor(false); }

/*
 * Blink!
 */
auto TextEditor::BlinkTimer::callback(TextEditor* te) -> bool {
    te->cursorVisible = !te->cursorVisible;
    auto time = te->cursorVisible ? te->cursorBlinkingTimeOn : te->cursorBlinkingTimeOff;
    te->blinkTimer = gdk_threads_add_timeout(time, reinterpret_cast<GSourceFunc>(callback), te);

    te->repaintCursor();

    // Remove ourselves
    return false;
}

void TextEditor::setTextToPangoLayout(PangoLayout* pl) const {
    std::string_view preed(preeditString);

    if (!preed.empty()) {
        // When using an Input Method, we need to insert the preeditString into the text at the cursor location
        std::string txt = cloneWithInsertToStdString(this->buffer.get(), preed);

        int pos = getByteOffsetOfCursor(this->buffer.get());
        xoj::util::PangoAttrListSPtr attrlist(pango_attr_list_new(), xoj::util::adopt);
        pango_attr_list_splice(attrlist.get(), this->preeditAttrList.get(), pos, static_cast<int>(preed.length()));

        pango_layout_set_attributes(pl, attrlist.get());
        pango_layout_set_text(pl, txt.c_str(), static_cast<int>(txt.length()));
    } else {
        pango_layout_set_text(pl, cloneToCString(this->buffer.get()).get(), -1);
    }
}

auto TextEditor::computeBoundingBox() const -> Range {
    /*
     * NB: we cannot rely on Text::calcSize directly, since it would not take the size changes due to the IM
     * preeditString into account.
     */
    auto* textElement = this->getText();

    setTextToPangoLayout(this->layout.get());

    int w = 0;
    int h = 0;
    pango_layout_get_size(this->layout.get(), &w, &h);
    double width = (static_cast<double>(w)) / PANGO_SCALE;
    double height = (static_cast<double>(h)) / PANGO_SCALE;
    double x = textElement->getX();
    double y = textElement->getY();

    // Warning: width can be negative (e.g. for languages written from right to left)
    Range res(x, y);
    res.addPoint(x + width, y + height);
    return res;
}

void TextEditor::repaintEditor(bool sizeChanged) {
    Range dirtyRange(this->previousBoundingBox);
    if (sizeChanged) {
        this->previousBoundingBox = this->computeBoundingBox();
        dirtyRange = dirtyRange.unite(this->previousBoundingBox);
    }
    const double zoom = this->gui->getXournal()->getZoom();
    const double padding = (BORDER_WIDTH_IN_PIXELS + PADDING_IN_PIXELS) / zoom;
    dirtyRange.addPadding(padding);
    this->gui->repaintArea(dirtyRange.minX, dirtyRange.minY, dirtyRange.maxX, dirtyRange.maxY);
}

auto TextEditor::drawCursor(cairo_t* cr, double zoom) const -> xoj::util::Rectangle<double> {
    xoj::util::Rectangle<double> cursorBox;
    {  // Compute the bounding box of the active grapheme (i.e. the one just after the cursor)
        int offset = getByteOffsetOfCursor(this->buffer.get());
        if (this->preeditString && this->preeditCursor != 0) {
            const gchar* preeditText = this->preeditString.get();
            offset += static_cast<int>(g_utf8_offset_to_pointer(preeditText, preeditCursor) - preeditText);
        }
        PangoRectangle rect = {0};
        pango_layout_index_to_pos(this->layout.get(), offset, &rect);
        cursorBox = xoj::util::Rectangle<double>(rect.x, rect.y, rect.width, rect.height);
        cursorBox *= 1.0 / PANGO_SCALE;
    }

    if (!this->cursorOverwrite || cursorBox.width == 0.0) {
        // cursorBox.width == 0.0 happens when the cursor is at the end of the line
        cursorBox.width = 2.0 / zoom;
    }

    if (this->cursorVisible) {
        xoj::util::CairoSaveGuard saveGuard(cr);

        cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_rectangle(cr, cursorBox.x, cursorBox.y, cursorBox.width, cursorBox.height);
        cairo_fill(cr);
    }

    return cursorBox;
}

void TextEditor::paint(cairo_t* cr, double zoom) {
    GdkRGBA selectionColor = this->gui->getSelectionColor();

    cairo_save(cr);

    Util::cairo_set_source_rgbi(cr, this->text->getColor());

    double x0 = this->text->getX();
    double y0 = this->text->getY();
    cairo_translate(cr, x0, y0);

    // The cairo context might have changed. Update the pango layout
    pango_cairo_update_layout(cr, this->layout.get());
    // Workaround https://gitlab.gnome.org/GNOME/pango/-/issues/691
    pango_context_set_matrix(pango_layout_get_context(this->layout.get()), nullptr);

    this->setTextToPangoLayout(this->layout.get());

    if (!this->preeditString) {
        GtkTextIter start;
        GtkTextIter end;
        bool hasSelection = gtk_text_buffer_get_selection_bounds(this->buffer.get(), &start, &end);

        if (hasSelection) {
            auto selectionColorU16 = Util::GdkRGBA_to_ColorU16(selectionColor);
            PangoAttribute* attrib =
                    pango_attr_background_new(selectionColorU16.red, selectionColorU16.green, selectionColorU16.blue);
            attrib->start_index = static_cast<unsigned int>(getByteOffsetOfIterator(start));
            attrib->end_index = static_cast<unsigned int>(getByteOffsetOfIterator(end));

            xoj::util::PangoAttrListSPtr attrlist(pango_attr_list_new(), xoj::util::adopt);
            pango_attr_list_insert(attrlist.get(), attrib);  // attrlist takes ownership of attrib
            pango_layout_set_attributes(this->layout.get(), attrlist.get());
        } else {
            // remove all attributes
            PangoAttrList* attrlist = pango_attr_list_new();
            pango_layout_set_attributes(this->layout.get(), attrlist);
            pango_attr_list_unref(attrlist);
            attrlist = nullptr;
        }
    }

    pango_cairo_show_layout(cr, this->layout.get());

    {
        auto cursorBox = drawCursor(cr, zoom);

        // Notify the IM of the app's window and cursor position.
        double x1 = this->gui->getX();
        double y1 = this->gui->getY();
        GdkRectangle cursorRect;  // cursor position in window coordinates
        cursorRect.x = static_cast<int>(zoom * x0 + x1 + zoom * cursorBox.x);
        cursorRect.y = static_cast<int>(zoom * y0 + y1 + zoom * cursorBox.y);
        cursorRect.height = static_cast<int>(zoom * cursorBox.height);
        cursorRect.width = static_cast<int>(zoom * cursorBox.width);
        gtk_im_context_set_cursor_location(this->imContext.get(), &cursorRect);
    }

    cairo_restore(cr);

    /*
     * Draw the box around the text
     *
     * Set the line width independently of the zoom
     */
    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / zoom);
    gdk_cairo_set_source_rgba(cr, &selectionColor);

    int w = 0;
    int h = 0;
    pango_layout_get_size(this->layout.get(), &w, &h);
    double width = (static_cast<double>(w)) / PANGO_SCALE;
    double height = (static_cast<double>(h)) / PANGO_SCALE;

    cairo_rectangle(cr, x0 - PADDING_IN_PIXELS / zoom, y0 - PADDING_IN_PIXELS / zoom,
                    width + 2 * PADDING_IN_PIXELS / zoom, height + 2 * PADDING_IN_PIXELS / zoom);
    cairo_stroke(cr);

    this->text->setWidth(width);
    this->text->setHeight(height);
}
