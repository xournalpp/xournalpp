#include "TextEditor.h"

#include <cstring>  // for strcmp, size_t
#include <memory>   // for allocator, make_unique, __shared_p...
#include <string>   // for std::string()
#include <utility>  // for move

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_B, GDK_KEY_ISO_Enter, GDK_...
#include <glib-object.h>     // for g_object_get, g_object_unref, G_CA...
#include <gtk/gtk.h>         // for GtkPopover

#include "control/AudioController.h"
#include "control/Control.h"  // for Control
#include "control/settings/Settings.h"
#include "control/zoom/ZoomControl.h"  // for ZoomControl
#include "gui/GladeSearchpath.h"       // for GladeSearchPath
#include "gui/PageView.h"
#include "gui/XournalppCursor.h"              // for XournalppCursor
#include "gui/menus/TextEditorContextMenu.h"  // for TextEditorContextMenu
#include "model/Font.h"                       // for XojFont
#include "model/Text.h"                       // for Text
#include "model/XojPage.h"                    // for XojPage
#include "undo/DeleteUndoAction.h"
#include "undo/InsertUndoAction.h"
#include "undo/TextBoxUndoAction.h"
#include "undo/UndoRedoHandler.h"  // for UndoRedoHandler
#include "util/Assert.h"
#include "util/DispatchPool.h"
#include "util/Range.h"
#include "util/glib_casts.h"  // for wrap_for_once_v
#include "util/gtk4_helper.h"
#include "util/raii/CStringWrapper.h"
#include "util/safe_casts.h"                // for round_cast, as_unsigned
#include "view/overlays/TextEditionView.h"  // for TextEditionView

#include "TextEditorWidget.h"  // for gtk_xoj_int_txt_new

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
    while (gtk_text_iter_backward_line(&it)) {
        pos += gtk_text_iter_get_bytes_in_line(&it);
    }
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
    xoj_assert(byteIndex >= 0);
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
 * @brief Clone the buffer's content
 *
 * This is pretty inefficient: the text gets copied twice
 */
static auto cloneToStdString(GtkTextBuffer* buf) -> std::string { return cloneToCString(buf).get(); }

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

TextEditor::TextEditor(Control* control, XojPageView* pageView, GtkWidget* xournalWidget, double x, double y):
        control(control),
        page(pageView->getPage()),
        pageView(pageView),
        xournalWidget(xournalWidget),
        textWidget(gtk_xoj_int_txt_new(this), xoj::util::adopt),
        imContext(gtk_im_multicontext_new(), xoj::util::adopt),
        buffer(gtk_text_buffer_new(nullptr), xoj::util::adopt),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::TextEditionView>>()) {

    this->contextMenu = std::make_unique<TextEditorContextMenu>(control, this, pageView, xournalWidget);

    this->pageView->getZoomControl()->addZoomListener(this);

    this->initializeEditionAt(x, y);

    g_signal_connect(this->buffer.get(), "paste-done", G_CALLBACK(bufferPasteDoneCallback), this);

    {  // Get cursor blinking settings
        GtkSettings* settings = gtk_widget_get_settings(this->xournalWidget);
        g_object_get(settings, "gtk-cursor-blink", &this->cursorBlink, nullptr);
        if (this->cursorBlink) {
            int tmp = 0;
            g_object_get(settings, "gtk-cursor-blink-time", &tmp, nullptr);
            xoj_assert(tmp >= 0);
            auto cursorBlinkingPeriod = static_cast<unsigned int>(tmp);
            this->cursorBlinkingTimeOn = cursorBlinkingPeriod * CURSOR_ON_MULTIPLIER / CURSOR_DIVIDER;
            this->cursorBlinkingTimeOff = cursorBlinkingPeriod - this->cursorBlinkingTimeOn;
        }
    }

    gtk_im_context_set_client_widget(this->imContext.get(), this->xournalWidget);
    gtk_im_context_focus_in(this->imContext.get());

    g_signal_connect(this->imContext.get(), "commit", G_CALLBACK(iMCommitCallback), this);
    g_signal_connect(this->imContext.get(), "preedit-changed", G_CALLBACK(iMPreeditChangedCallback), this);
    g_signal_connect(this->imContext.get(), "retrieve-surrounding", G_CALLBACK(iMRetrieveSurroundingCallback), this);
    g_signal_connect(this->imContext.get(), "delete-surrounding", G_CALLBACK(imDeleteSurroundingCallback), this);

    if (this->originalTextElement) {
        // If editing a preexisting text, put the cursor at the right location
        this->mousePressed(x - textElement->getX(), y - textElement->getY());
    } else if (this->cursorBlink) {
        BlinkTimer::callback(this);
    } else {
        this->cursorVisible = true;
    }
}

TextEditor::~TextEditor() {
    gtk_im_context_focus_out(this->imContext.get());

    this->xournalWidget = nullptr;
    control->setCopyCutEnabled(false);

    this->contentsChanged(true);

    finalizeEdition();

    this->pageView->getZoomControl()->removeZoomListener(this);
}

auto TextEditor::getViewPool() const -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::TextEditionView>>& {
    return viewPool;
}

auto TextEditor::getTextElement() const -> Text* { return this->textElement.get(); }

bool TextEditor::bufferEmpty() const { return gtk_text_buffer_get_char_count(this->buffer.get()) == 0; }

void TextEditor::replaceBufferContent(const std::string& text) {
    gtk_text_buffer_set_text(this->buffer.get(), text.c_str(), -1);

    GtkTextIter first = {nullptr};
    gtk_text_buffer_get_iter_at_offset(this->buffer.get(), &first, 0);
    gtk_text_buffer_place_cursor(this->buffer.get(), &first);
    this->layoutStatus = LayoutStatus::NEEDS_COMPLETE_UPDATE;
    this->cursorBox = computeCursorBox();
}

void TextEditor::setColor(Color color) {
    this->textElement->setColor(color);
    repaintEditor(false);
}

void TextEditor::setFont(XojFont font) {
    this->textElement->setFont(font);
    afterFontChange();
}

void TextEditor::afterFontChange() {
    this->textElement->updatePangoFont(this->layout.get());
    this->computeVirtualCursorPosition();
    this->repaintEditor();
}

void TextEditor::iMCommitCallback(GtkIMContext* context, const gchar* str, TextEditor* te) {
    gtk_text_buffer_begin_user_action(te->buffer.get());

    bool hadSelection = gtk_text_buffer_get_has_selection(te->buffer.get());
    te->deleteSelection();

    GtkTextIter curser = getIteratorAtCursor(te->buffer.get());
    int spos = gtk_text_iter_get_offset(&curser);
    int delta = std::string(str).length();

    if (!strcmp(str, "\n")) {
        if (!gtk_text_buffer_insert_interactive_at_cursor(te->buffer.get(), "\n", 1, true)) {
            gtk_widget_error_bell(te->xournalWidget);
        } else {
            te->updateTextAttributesPos(spos, 0, delta);
            te->contentsChanged(true);
        }
    } else {
        if (!hadSelection && te->cursorOverwrite) {
            auto insert = getIteratorAtCursor(te->buffer.get());
            if (!gtk_text_iter_ends_line(&insert)) {
                te->deleteFromCursor(GTK_DELETE_CHARS, 1);
            } else {
                te->updateTextAttributesPos(spos, 0, delta);
            }
        }

        if (!gtk_text_buffer_insert_interactive_at_cursor(te->buffer.get(), str, -1, true)) {
            gtk_widget_error_bell(te->xournalWidget);
        } else {
            te->updateTextAttributesPos(spos, 0, delta);
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
    auto keymap = gdk_keymap_get_for_display(gdk_display_get_default());
    GdkModifierType consumed;
    /*
    According to https://docs.gtk.org/gdk3/method.Keymap.translate_keyboard_state.html
    consumed modifiers should be masked out. For instance, on a US keyboard, the plus symbol is shifted, so when
    comparing a key press to a <Control>plus accelerator <Shift> should be masked out.
    */
    gdk_keymap_translate_keyboard_state(keymap, event->hardware_keycode, static_cast<GdkModifierType>(event->state),
                                        event->group, nullptr, nullptr, nullptr, &consumed);

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
    } else if ((event->state & ~consumed & modifiers) == GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_b || event->keyval == GDK_KEY_B) {
            toggleBoldFace();
            return true;
        }
        if (event->keyval == GDK_KEY_plus || event->keyval == GDK_KEY_KP_Add) {
            increaseFontSize();
            return true;
        }
        if (event->keyval == GDK_KEY_minus || event->keyval == GDK_KEY_KP_Subtract) {
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
        Settings* settings = control->getSettings();
        if (!settings->getUseSpacesAsTab()) {
            iMCommitCallback(nullptr, "\t", this);
        } else {
            std::string indent(static_cast<size_t>(settings->getNumberOfSpacesForTab()), ' ');
            iMCommitCallback(nullptr, indent.c_str(), this);
        }
        obscure = true;
        retval = true;
    } else {
        retval = false;
    }

    if (obscure) {
        control->getCursor()->setInvisible(true);
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
    repaintCursorAfterChange();
}

/**
 * I know it's a bit rough and duplicated
 * Improve that later on...
 */
void TextEditor::decreaseFontSize() {
    XojFont& font = textElement->getFont();
    if (double size = font.getSize(); size > 1) {
        font.setSize(font.getSize() - 1);
        afterFontChange();
    }
}

void TextEditor::increaseFontSize() {
    XojFont& font = textElement->getFont();
    font.setSize(font.getSize() + 1);
    afterFontChange();
}

void TextEditor::toggleBoldFace() {
    // get the current/used font
    XojFont& font = textElement->getFont();
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

    // Selection highlighting is handled through Pango attributes
    this->layoutStatus = LayoutStatus::NEEDS_ATTRIBUTES_UPDATE;
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
}

void TextEditor::findPos(GtkTextIter* iter, double xPos, double yPos) const {
    int index = 0;
    int trailing = 0;
    pango_layout_xy_to_index(this->getUpToDateLayout(), round_cast<int>(xPos * PANGO_SCALE),
                             round_cast<int>(yPos * PANGO_SCALE), &index, &trailing);
    /*
     * trailing is non-zero iff the abscissa is past the middle of the grapheme.
     * In this case, it contains the length of the grapheme in utf8 char count.
     * This way, we put the cursor after the grapheme when clicking past the middle of the grapheme.
     */
    *iter = getIteratorAtByteOffset(this->buffer.get(), index);
    gtk_text_iter_forward_chars(iter, trailing);
}

void TextEditor::updateTextElementContent() { this->textElement->setText(cloneToStdString(this->buffer.get())); }

void TextEditor::contentsChanged(bool forceCreateUndoAction) {
    // Todo: Reinstate text edition undo stack
    this->layoutStatus = LayoutStatus::NEEDS_COMPLETE_UPDATE;
    this->computeVirtualCursorPosition();
    this->contextMenu->reposition();
}

void TextEditor::markPos(double x, double y, bool extendSelection) {
    GtkTextIter newplace = getIteratorAtCursor(this->buffer.get());

    findPos(&newplace, x, y);

    // call moveCursor() even if the cursor hasn't moved, since it cancels the selection
    moveCursor(&newplace, extendSelection);
    computeVirtualCursorPosition();
}

void TextEditor::mousePressed(double x, double y) {
    this->mouseDown = true;
    // Todo select if SHIFT is pressed
    markPos(x, y, false);
}

void TextEditor::mouseMoved(double x, double y) {
    if (this->mouseDown) {
        markPos(x, y, true);

        if (this->hasSelection()) {
            auto selection = this->getCurrentSelection().value();
            if (std::get<0>(selection) == std::get<0>(this->previousSelection) &&
                std::get<1>(selection) == std::get<1>(this->previousSelection)) {
                return;
            }
            GSList* attribs = pango_attr_list_get_attributes(pango_layout_get_attributes(this->getUpToDateLayout()));
            std::list<PangoAttribute*> filteredList = {};
            for (int i = 0; i < g_slist_length(attribs); i++) {
                PangoAttribute* attrib = (PangoAttribute*)g_slist_nth_data(attribs, i);
                if (attrib->start_index <= std::get<0>(selection) && attrib->end_index >= std::get<1>(selection)) {
                    filteredList.push_back(attrib);
                } else {
                    pango_attribute_destroy(attrib);
                }
            }
            g_slist_free(attribs);
            this->contextMenu->setAttributes(filteredList);
            this->contextMenu->showFullMenu();
            this->previousSelection = selection;
        } else {
            this->contextMenu->showReducedMenu();
        }
    }
}

void TextEditor::mouseReleased() {
    this->mouseDown = false;

    if (this->hasSelection()) {
        auto selection = this->getCurrentSelection().value();
        if (std::get<0>(selection) == std::get<0>(this->previousSelection) &&
            std::get<1>(selection) == std::get<1>(this->previousSelection)) {
            return;
        }
        GSList* attribs = pango_attr_list_get_attributes(pango_layout_get_attributes(this->getUpToDateLayout()));
        std::list<PangoAttribute*> filteredList = {};
        for (int i = 0; i < g_slist_length(attribs); i++) {
            PangoAttribute* attrib = (PangoAttribute*)g_slist_nth_data(attribs, i);
            if (attrib->start_index <= std::get<0>(selection) && attrib->end_index >= std::get<1>(selection)) {
                filteredList.push_back(attrib);
            } else {
                pango_attribute_destroy(attrib);
            }
        }
        g_slist_free(attribs);
        this->contextMenu->setAttributes(filteredList);
        this->contextMenu->showFullMenu();
        this->previousSelection = selection;
    } else {
        this->contextMenu->showReducedMenu();
    }
}

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
    pango_layout_index_to_pos(this->getUpToDateLayout(), offset, &rect);
    this->virtualCursorAbscissa = rect.x;
}

void TextEditor::moveCursor(const GtkTextIter* newLocation, gboolean extendSelection) {
    bool selectionChanged = true;
    if (extendSelection) {
        if (auto oldLoc = getIteratorAtCursor(this->buffer.get()); gtk_text_iter_equal(newLocation, &oldLoc)) {
            // Nothing changed
            return;
        }
        // this sets the text selection
        gtk_text_buffer_move_mark_by_name(this->buffer.get(), "insert", newLocation);
        control->setCopyCutEnabled(true);
    } else {
        // if !extendSelection, we clear the selection even if the cursor does not move
        selectionChanged = gtk_text_buffer_get_has_selection(this->buffer.get());
        gtk_text_buffer_place_cursor(this->buffer.get(), newLocation);
        control->setCopyCutEnabled(false);
    }

    if (this->cursorBlink) {
        // Whenever the cursor moves, the blinking cycle restarts from the start (i.e. the cursor is first shown).
        this->cursorVisible = false;  // Will be toggled to true by BlinkTimer::callback before the repaint
        BlinkTimer::callback(this);
    }

    if (selectionChanged) {
        // The selection background color is set through Pango attributes
        this->layoutStatus = LayoutStatus::NEEDS_ATTRIBUTES_UPDATE;
        // Repaint the entire box. Computing the exact area that was (un)selected would be better but complicated
        this->repaintEditor(false);
    } else {
        repaintCursorAfterChange();
    }
}

void TextEditor::updateCursorBox() {
    this->cursorBox = computeCursorBox();

    if (!viewPool->empty()) {
        // Inform the IM of the cursor location (for word selection popup's location)
        // We use the first view as the main view, as far as the IM is concerned
        auto box = viewPool->front().toWindowCoordinates(xoj::util::Rectangle<double>(this->cursorBox));

        GdkRectangle cursorRect;  // cursor position in window coordinates
        cursorRect.x = static_cast<int>(box.x);
        cursorRect.y = static_cast<int>(box.y);
        cursorRect.height = static_cast<int>(box.height);
        cursorRect.width = static_cast<int>(box.width);
        gtk_im_context_set_cursor_location(this->imContext.get(), &cursorRect);
    }
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
        if (this->deleteSelection()) {
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
        int spos = gtk_text_iter_get_offset(&start);
        int delta = gtk_text_iter_get_offset(&end) - spos;
        gtk_text_buffer_begin_user_action(this->buffer.get());

        if (!gtk_text_buffer_delete_interactive(this->buffer.get(), &start, &end, true)) {
            gtk_widget_error_bell(this->xournalWidget);
        }

        gtk_text_buffer_end_user_action(this->buffer.get());
        this->updateTextAttributesPos(spos, delta, 0);
    } else {
        gtk_widget_error_bell(this->xournalWidget);
    }

    this->contentsChanged();
    this->repaintEditor();
}

void TextEditor::backspace() {

    resetImContext();

    // Backspace deletes the selection, if one exists
    if (this->deleteSelection()) {
        return;
    }

    GtkTextIter insert = getIteratorAtCursor(this->buffer.get());
    int spos = gtk_text_iter_get_offset(&insert);

    if (gtk_text_buffer_backspace(this->buffer.get(), &insert, true, true)) {
        this->updateTextAttributesPos(spos - 1, 1, 0);
        this->contentsChanged();
        this->repaintEditor();
    } else {
        gtk_widget_error_bell(this->xournalWidget);
    }
}

bool TextEditor::deleteSelection() {
    GtkTextIter start, end;
    if (gtk_text_buffer_get_selection_bounds(this->buffer.get(), &start, &end)) {
        int spos = gtk_text_iter_get_offset(&start);
        int delta = gtk_text_iter_get_offset(&end) - spos;
        if (gtk_text_buffer_delete_selection(this->buffer.get(), true, true)) {
            this->updateTextAttributesPos(spos, delta, 0);
            this->contentsChanged();
            this->repaintEditor();
            return true;
        }
    };
    return false;
}

void TextEditor::copyToClipboard() const {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget);
    gtk_text_buffer_copy_clipboard(this->buffer.get(), clipboard);
}

void TextEditor::cutToClipboard() {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget);
    gtk_text_buffer_cut_clipboard(this->buffer.get(), clipboard, true);

    this->contentsChanged(true);
    this->repaintEditor();
}

void TextEditor::pasteFromClipboard() {
    GtkClipboard* clipboard = gtk_widget_get_clipboard(this->xournalWidget);
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

/*
 * Blink!
 */
auto TextEditor::BlinkTimer::callback(TextEditor* te) -> bool {
    te->cursorVisible = !te->cursorVisible;
    auto time = te->cursorVisible ? te->cursorBlinkingTimeOn : te->cursorBlinkingTimeOff;
    te->blinkTimer = gdk_threads_add_timeout(time, xoj::util::wrap_for_once_v<callback>, te);

    Range dirtyRange = te->cursorBox;
    dirtyRange.translate(te->textElement->getX(), te->textElement->getY());
    te->viewPool->dispatch(xoj::view::TextEditionView::FLAG_DIRTY_REGION, dirtyRange);

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
        setAttributesToPangoLayout(pl);
        pango_layout_set_text(pl, cloneToCString(this->buffer.get()).get(), -1);
    }
}

Color TextEditor::getSelectionColor() const { return this->control->getSettings()->getSelectionColor(); }

void TextEditor::setAttributesToPangoLayout(PangoLayout* pl) const {
    xoj::util::PangoAttrListSPtr attrlist = this->textElement->getAttributeList();

    /*GtkTextIter start;
    GtkTextIter end;
    bool hasSelection = gtk_text_buffer_get_selection_bounds(this->buffer.get(), &start, &end);

    if (hasSelection) {
        auto selectionColorU16 = Util::argb_to_ColorU16(this->getSelectionColor());

        gtk_text_iter_order(&start, &end);

        PangoAttribute* attrib =
                pango_attr_background_new(selectionColorU16.red, selectionColorU16.green, selectionColorU16.blue);
        attrib->start_index = static_cast<unsigned int>(getByteOffsetOfIterator(start));
        attrib->end_index = static_cast<unsigned int>(getByteOffsetOfIterator(end));

        PangoAttribute* attrib2 = pango_attr_background_alpha_new(int(double(UINT16_MAX) * 0.5));
        attrib2->start_index = static_cast<unsigned int>(getByteOffsetOfIterator(start));
        attrib2->end_index = static_cast<unsigned int>(getByteOffsetOfIterator(end));
        pango_attr_list_insert_before(attrlist.get(), attrib2);


        pango_attr_list_insert_before(attrlist.get(), attrib);  // attrlist takes ownership of attrib
    }*/


    pango_layout_set_attributes(pl, attrlist.get());

    PangoAlignment align = static_cast<PangoAlignment>(this->textElement->getAlignment());
    pango_layout_set_alignment(pl, align);
}

auto TextEditor::computeBoundingBox() const -> Range {
    /*
     * NB: we cannot rely on Text::calcSize directly, since it would not take the size changes due to the IM
     * preeditString into account.
     */
    int w = 0;
    int h = 0;
    pango_layout_get_size(getUpToDateLayout(), &w, &h);
    double width = (static_cast<double>(w)) / PANGO_SCALE;
    double height = (static_cast<double>(h)) / PANGO_SCALE;
    double x = textElement->getX();
    double y = textElement->getY();

    // Warning: width can be negative (e.g. for languages written from right to left)
    Range res(x, y);
    res.addPoint(x + width, y + height);
    return res;
}

auto TextEditor::getUpToDateLayout() const -> PangoLayout* {
    switch (layoutStatus) {
        case LayoutStatus::NEEDS_COMPLETE_UPDATE:
            setTextToPangoLayout(this->layout.get());
            break;
        case LayoutStatus::NEEDS_ATTRIBUTES_UPDATE:
            setAttributesToPangoLayout(this->layout.get());
            break;
        case LayoutStatus::UP_TO_DATE:
            break;
    }
    layoutStatus = LayoutStatus::UP_TO_DATE;
    return this->layout.get();
}

auto TextEditor::getCursorBox() const -> const Range& { return this->cursorBox; }

auto TextEditor::getContentBoundingBox() const -> const Range& { return this->previousBoundingBox; }

bool TextEditor::isCursorVisible() const { return cursorVisible; }

auto TextEditor::computeCursorBox() const -> Range {
    // Compute the bounding box of the active grapheme (i.e. the one just after the cursor)
    int offset = getByteOffsetOfCursor(this->buffer.get());
    if (this->preeditString && this->preeditCursor != 0) {
        const gchar* preeditText = this->preeditString.get();
        offset += static_cast<int>(g_utf8_offset_to_pointer(preeditText, preeditCursor) - preeditText);
    }
    PangoRectangle rect = {0};
    pango_layout_index_to_pos(getUpToDateLayout(), offset, &rect);
    const double ratio = 1.0 / PANGO_SCALE;

    // Warning: rect.width could be negative (e.g. for languages written from right to left).
    Range res(rect.x * ratio, rect.y * ratio);
    res.addPoint((rect.x + (cursorOverwrite ? rect.width : 0.0)) * ratio, (rect.y + rect.height) * ratio);
    return res;
}

void TextEditor::repaintEditor(bool sizeChanged) {
    Range dirtyRange(this->previousBoundingBox);
    if (sizeChanged) {
        this->previousBoundingBox = this->computeBoundingBox();
        dirtyRange = dirtyRange.unite(this->previousBoundingBox);
    }
    this->updateCursorBox();
    this->viewPool->dispatch(xoj::view::TextEditionView::FLAG_DIRTY_REGION, dirtyRange);
}

void TextEditor::repaintCursorAfterChange() {
    Range dirtyRange = this->cursorBox;
    this->updateCursorBox();
    dirtyRange = dirtyRange.unite(this->cursorBox);
    dirtyRange.translate(this->textElement->getX(), this->textElement->getY());
    this->viewPool->dispatch(xoj::view::TextEditionView::FLAG_DIRTY_REGION, dirtyRange);
}

void TextEditor::finalizeEdition() {
    Layer* layer = this->page->getSelectedLayer();
    UndoRedoHandler* undo = this->control->getUndoRedoHandler();

    this->control->setFontSelected(this->control->getSettings()->getFont());

    if (this->bufferEmpty()) {
        this->contextMenu->hide();
        // Delete the edited element from layer
        if (originalTextElement) {
            auto eraseDeleteUndoAction = std::make_unique<DeleteUndoAction>(page, true);
            auto [orig, elementIndex] = layer->removeElement(originalTextElement);
            xoj_assert(elementIndex != Element::InvalidIndex);
            eraseDeleteUndoAction->addElement(layer, std::move(orig), elementIndex);
            undo->addUndoAction(std::move(eraseDeleteUndoAction));
            originalTextElement = nullptr;
        }
        this->viewPool->dispatchAndClear(xoj::view::TextEditionView::FINALIZATION_REQUEST, this->previousBoundingBox);
        return;
    }

    this->updateTextElementContent();
    if (originalTextElement) {
        // Modifying a preexisting element
        this->viewPool->dispatchAndClear(xoj::view::TextEditionView::FINALIZATION_REQUEST, this->previousBoundingBox);

        this->originalTextElement->setInEditing(false);

        auto [orig, _] = layer->removeElement(this->originalTextElement);
        auto ptr = this->textElement.get();
        layer->addElement(std::move(this->textElement));

        this->page->fireElementChanged(ptr);

        undo->addUndoAction(std::make_unique<TextBoxUndoAction>(this->page, layer, ptr, std::move(orig)));
    } else {
        // Creating a new element
        auto ptr = this->textElement.get();
        layer->addElement(std::move(this->textElement));
        this->viewPool->dispatchAndClear(xoj::view::TextEditionView::FINALIZATION_REQUEST, this->previousBoundingBox);
        this->page->fireElementChanged(ptr);
        undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, ptr));
    }
    this->contextMenu->hide();
}

void TextEditor::initializeEditionAt(double x, double y) {
    // Is there already a textfield?
    Text* text = nullptr;

    // Should we reverse this loop to select the most recent text rather than the oldest?
    for (auto&& e: this->page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_TEXT) {
            GdkRectangle matchRect = {gint(x), gint(y), 1, 1};
            if (e->intersectsArea(&matchRect)) {
                text = dynamic_cast<Text*>(e.get());
                break;
            }
        }
    }

    if (text == nullptr) {
        ToolHandler* h = this->control->getToolHandler();
        this->textElement = std::make_unique<Text>();
        this->textElement->setColor(h->getColor());
        this->textElement->setFont(control->getSettings()->getFont());
        this->textElement->setX(x);
        this->textElement->setY(y - this->textElement->getElementHeight() / 2);

        if (auto audioController = control->getAudioController(); audioController && audioController->isRecording()) {
            fs::path audioFilename = audioController->getAudioFilename();
            size_t sttime = audioController->getStartTime();
            size_t milliseconds = (as_unsigned(g_get_monotonic_time() / 1000) - sttime);
            this->textElement->setTimestamp(milliseconds);
            this->textElement->setAudioFilename(audioFilename);
        }
        this->originalTextElement = nullptr;
    } else {
        this->control->setFontSelected(text->getFont());
        this->originalTextElement = text;

        this->textElement = text->cloneText();

        text->setInEditing(true);
        this->page->fireElementChanged(text);
    }
    this->layout = this->textElement->createPangoLayout();
    this->previousBoundingBox = Range(this->textElement->boundingRect());
    this->replaceBufferContent(this->textElement->getText());

    this->contextMenu->show();
}

void TextEditor::zoomChanged() { this->contextMenu->reposition(); }

void TextEditor::setTextAlignment(TextAlignment align) {
    this->textElement->setAlignment(align);
    this->layoutStatus = LayoutStatus::NEEDS_ATTRIBUTES_UPDATE;
    this->repaintEditor();
}

std::optional<std::tuple<int, int>> TextEditor::getCurrentSelection() const {
    GtkTextIter start, end;
    bool hasSelection = gtk_text_buffer_get_selection_bounds(this->buffer.get(), &start, &end);
    return hasSelection ? std::make_optional(
                                  std::make_tuple(gtk_text_iter_get_offset(&start), gtk_text_iter_get_offset(&end))) :
                          std::nullopt;
}

bool TextEditor::hasSelection() const { return gtk_text_buffer_get_has_selection(this->buffer.get()); }

void TextEditor::updateTextAttributesPos(int pos, int del, int add) {
    this->textElement->updateTextAttributesPosition(pos, del, add);
}

void TextEditor::setBackgroundColorInline(GdkRGBA color) {
    std::tuple<int, int> selection = getCurrentSelection().value_or(
            std::make_tuple(PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING, PANGO_ATTR_INDEX_TO_TEXT_END));

    PangoAttribute* attrib = pango_attr_background_new(guint16(double(UINT16_MAX) * color.red),
                                                       guint16(double(UINT16_MAX) * color.green),
                                                       guint16(double(UINT16_MAX) * color.blue));
    attrib->start_index = std::get<0>(selection);
    attrib->end_index = std::get<1>(selection);
    this->textElement->addAttribute(attrib);

    if (color.alpha != 1.0) {
        PangoAttribute* alpha = pango_attr_background_alpha_new(guint16(double(UINT16_MAX) * color.alpha) + 1);
        alpha->start_index = std::get<0>(selection);
        alpha->end_index = std::get<1>(selection);
        this->textElement->addAttribute(alpha);
    }

    this->layoutStatus = LayoutStatus::NEEDS_ATTRIBUTES_UPDATE;
    this->repaintEditor();
}

void TextEditor::setFontInline(PangoFontDescription* font) {
    std::tuple<int, int> selection = getCurrentSelection().value_or(
            std::make_tuple(PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING, PANGO_ATTR_INDEX_TO_TEXT_END));
    PangoAttribute* attrib = pango_attr_font_desc_new(font);
    attrib->start_index = std::get<0>(selection);
    attrib->end_index = std::get<1>(selection);
    this->textElement->addAttribute(attrib);
    this->layoutStatus = LayoutStatus::NEEDS_COMPLETE_UPDATE;
    this->repaintEditor();
}

void TextEditor::setFontColorInline(GdkRGBA color) {
    std::tuple<int, int> selection = getCurrentSelection().value_or(
            std::make_tuple(PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING, PANGO_ATTR_INDEX_TO_TEXT_END));

    PangoAttribute* attrib = pango_attr_foreground_new(guint16(double(UINT16_MAX) * color.red),
                                                       guint16(double(UINT16_MAX) * color.green),
                                                       guint16(double(UINT16_MAX) * color.blue));
    attrib->start_index = std::get<0>(selection);
    attrib->end_index = std::get<1>(selection);

    this->textElement->addAttribute(attrib);

    this->layoutStatus = LayoutStatus::NEEDS_ATTRIBUTES_UPDATE;
    this->repaintEditor();
}

void TextEditor::addTextAttributeInline(PangoAttribute* attrib) {
    std::tuple<int, int> selection = getCurrentSelection().value_or(
            std::make_tuple(PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING, PANGO_ATTR_INDEX_TO_TEXT_END));
    attrib->start_index = std::get<0>(selection);
    attrib->end_index = std::get<1>(selection);
    this->textElement->addAttribute(attrib);
    this->layoutStatus = LayoutStatus::NEEDS_ATTRIBUTES_UPDATE;
    this->repaintEditor();
}

void TextEditor::clearAttributes() {
    this->textElement->clearAttributes();
    this->layoutStatus = LayoutStatus::NEEDS_COMPLETE_UPDATE;
    this->repaintEditor();
}
