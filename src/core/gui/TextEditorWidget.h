/*
 * Xournal++
 *
 * Dummy widget needed for text editor event handling
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#define GTK_TYPE_XOJ_INT_TXT (gtk_xoj_int_txt_get_type())
#define GTK_XOJ_INT_TXT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_XOJ_INT_TXT, GtkXojIntTxt))
#define GTK_XOJ_INT_TXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_XOJ_INT_TXT, GtkXojIntTxtClass))
#define GTK_IS_XOJ_INT_TXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_XOJ_INT_TXT))
#define GTK_IS_XOJ_INT_TXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_XOJ_INT_TXT))
#define GTK_XOJ_INT_TXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_XOJ_INT_TXT, GtkXojIntTxtClass))

typedef struct _GtkXojIntTxt GtkXojIntTxt;
typedef struct _GtkXojIntTxtClass GtkXojIntTxtClass;

struct _GtkXojIntTxt {
    GtkWidget widget;
    TextEditor* te;
};

struct _GtkXojIntTxtClass {
    GtkWidgetClass parent_class;
};

GType gtk_xoj_int_txt_get_type(void);
GtkWidget* gtk_xoj_int_txt_new(TextEditor* te);

G_DEFINE_TYPE(GtkXojIntTxt, gtk_xoj_int_txt, GTK_TYPE_WIDGET)

static void gtk_invisible_realize(GtkWidget* widget);
static void gtk_invisible_style_set(GtkWidget* widget, GtkStyle* previous_style);
static void gtk_invisible_show(GtkWidget* widget);
static void gtk_invisible_size_allocate(GtkWidget* widget, GtkAllocation* allocation);
static GObject* gtk_invisible_constructor(GType type, guint n_construct_properties,
                                          GObjectConstructParam* construct_params);

enum {
    SELECT_ALL,
    DELETE_FROM_CURSOR,
    BACKSPACE,
    CUT_CLIPBOARD,
    COPY_CLIPBOARD,
    PASTE_CLIPBOARD,
    TOGGLE_OVERWRITE,
    MOVE_CURSOR,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

class TextEditorCallbacks {
public:
    static void gtk_xoj_int_txt_select_all(GtkWidget* widget) {
        GtkXojIntTxt* txt = GTK_XOJ_INT_TXT(widget);
        txt->te->selectAtCursor(TextEditor::SelectType::ALL);
    }

    static void gtk_xoj_int_txt_move_cursor(GtkWidget* widget, gint step, gint bitmask) {
        GtkXojIntTxt* txt = GTK_XOJ_INT_TXT(widget);
        txt->te->moveCursor((GtkMovementStep)step, (bitmask & 2) ? 1 : -1, bitmask & 1);
    }

    static void gtk_xoj_int_txt_delete_from_cursor(GtkWidget* widget, gint type, gint count) {
        GtkXojIntTxt* txt = GTK_XOJ_INT_TXT(widget);
        txt->te->deleteFromCursor((GtkDeleteType)type, count);
    }

    static void gtk_xoj_int_txt_backspace(GtkWidget* widget) {
        GtkXojIntTxt* txt = GTK_XOJ_INT_TXT(widget);
        txt->te->backspace();
    }

    static void gtk_xoj_int_txt_cut_clipboard(GtkWidget* widget) {
        GtkXojIntTxt* txt = GTK_XOJ_INT_TXT(widget);
        txt->te->cutToClipboard();
    }

    static void gtk_xoj_int_txt_copy_clipboard(GtkWidget* widget) {
        GtkXojIntTxt* txt = GTK_XOJ_INT_TXT(widget);
        txt->te->copyToClipboard();
    }

    static void gtk_xoj_int_txt_paste_clipboard(GtkWidget* widget) {
        GtkXojIntTxt* txt = GTK_XOJ_INT_TXT(widget);
        txt->te->pasteFromClipboard();
    }

    static void gtk_xoj_int_txt_toggle_overwrite(GtkWidget* widget) {
        GtkXojIntTxt* txt = GTK_XOJ_INT_TXT(widget);
        txt->te->toggleOverwrite();
    }
};

static void add_move_binding(GtkBindingSet* binding_set, guint keyval, guint modmask, int step, gint count) {
    g_assert((modmask & GDK_SHIFT_MASK) == 0);

    gint bitmask = 0;
    if (count == 1) {
        bitmask |= 2;
    }

    gtk_binding_entry_add_signal(binding_set, keyval, (GdkModifierType)modmask, "move-cursor", 2, G_TYPE_INT, step,
                                 G_TYPE_INT, bitmask);

    /* Selection-extending version */
    gtk_binding_entry_add_signal(binding_set, keyval, (GdkModifierType)(modmask | GDK_SHIFT_MASK), "move-cursor", 2,
                                 G_TYPE_INT, step, G_TYPE_INT, bitmask | 1);
}

static void gtk_xoj_int_txt_class_init(GtkXojIntTxtClass* klass) {
    GObjectClass* gobject_class;
    GtkWidgetClass* widget_class;

    widget_class = (GtkWidgetClass*)klass;
    gobject_class = (GObjectClass*)klass;

    widget_class->realize = gtk_invisible_realize;
    widget_class->style_set = gtk_invisible_style_set;
    widget_class->show = gtk_invisible_show;
    widget_class->size_allocate = gtk_invisible_size_allocate;

    gobject_class->constructor = gtk_invisible_constructor;

    /**
     * GtkTextView::select-all:
     * @text_view: the object which received the signal
     * @select: %true to select, %false to unselect
     *
     * The ::select-all signal is a
     * <link linkend="keybinding-signals">keybinding signal</link>
     * which gets emitted to select or unselect the complete
     * contents of the text view.
     *
     * The default bindings for this signal are Ctrl-a and Ctrl-/
     * for selecting and Shift-Ctrl-a and Ctrl-\ for unselecting.
     */
    signals[SELECT_ALL] = g_signal_new_class_handler("select-all", G_OBJECT_CLASS_TYPE(widget_class),
                                                     (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                                                     G_CALLBACK(TextEditorCallbacks::gtk_xoj_int_txt_select_all),
                                                     nullptr, nullptr, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[MOVE_CURSOR] = g_signal_new_class_handler(
            "move-cursor", G_OBJECT_CLASS_TYPE(widget_class), (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            G_CALLBACK(TextEditorCallbacks::gtk_xoj_int_txt_move_cursor), nullptr, nullptr, nullptr, G_TYPE_NONE, 2,
            G_TYPE_INT, G_TYPE_INT);

    signals[DELETE_FROM_CURSOR] =
            g_signal_new_class_handler("delete-from-cursor", G_OBJECT_CLASS_TYPE(widget_class),
                                       (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                                       G_CALLBACK(TextEditorCallbacks::gtk_xoj_int_txt_delete_from_cursor), nullptr,
                                       nullptr, nullptr, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    signals[BACKSPACE] = g_signal_new_class_handler("backspace", G_OBJECT_CLASS_TYPE(widget_class),
                                                    (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                                                    G_CALLBACK(TextEditorCallbacks::gtk_xoj_int_txt_backspace), nullptr,
                                                    nullptr, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[CUT_CLIPBOARD] = g_signal_new_class_handler(
            "cut-clipboard", G_OBJECT_CLASS_TYPE(widget_class), (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            G_CALLBACK(TextEditorCallbacks::gtk_xoj_int_txt_cut_clipboard), nullptr, nullptr,
            g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[COPY_CLIPBOARD] = g_signal_new_class_handler(
            "copy-clipboard", G_OBJECT_CLASS_TYPE(widget_class), (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            G_CALLBACK(TextEditorCallbacks::gtk_xoj_int_txt_copy_clipboard), nullptr, nullptr,
            g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[PASTE_CLIPBOARD] = g_signal_new_class_handler(
            "paste-clipboard", G_OBJECT_CLASS_TYPE(widget_class), (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            G_CALLBACK(TextEditorCallbacks::gtk_xoj_int_txt_paste_clipboard), nullptr, nullptr,
            g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[TOGGLE_OVERWRITE] = g_signal_new_class_handler(
            "toggle-overwrite", G_OBJECT_CLASS_TYPE(widget_class), (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            G_CALLBACK(TextEditorCallbacks::gtk_xoj_int_txt_toggle_overwrite), nullptr, nullptr,
            g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////

    /*
     * Key bindings
     */
    GtkBindingSet* binding_set = gtk_binding_set_by_class(klass);

    /* Moving the insertion point */
    add_move_binding(binding_set, GDK_KEY_Right, 0, GTK_MOVEMENT_VISUAL_POSITIONS, 1);
    add_move_binding(binding_set, GDK_KEY_KP_Right, 0, GTK_MOVEMENT_VISUAL_POSITIONS, 1);

    add_move_binding(binding_set, GDK_KEY_Left, 0, GTK_MOVEMENT_VISUAL_POSITIONS, -1);
    add_move_binding(binding_set, GDK_KEY_KP_Left, 0, GTK_MOVEMENT_VISUAL_POSITIONS, -1);

    add_move_binding(binding_set, GDK_KEY_Right, GDK_CONTROL_MASK, GTK_MOVEMENT_WORDS, 1);
    add_move_binding(binding_set, GDK_KEY_KP_Right, GDK_CONTROL_MASK, GTK_MOVEMENT_WORDS, 1);

    add_move_binding(binding_set, GDK_KEY_Left, GDK_CONTROL_MASK, GTK_MOVEMENT_WORDS, -1);
    add_move_binding(binding_set, GDK_KEY_KP_Left, GDK_CONTROL_MASK, GTK_MOVEMENT_WORDS, -1);

    add_move_binding(binding_set, GDK_KEY_Up, 0, GTK_MOVEMENT_DISPLAY_LINES, -1);
    add_move_binding(binding_set, GDK_KEY_KP_Up, 0, GTK_MOVEMENT_DISPLAY_LINES, -1);

    add_move_binding(binding_set, GDK_KEY_Down, 0, GTK_MOVEMENT_DISPLAY_LINES, 1);
    add_move_binding(binding_set, GDK_KEY_KP_Down, 0, GTK_MOVEMENT_DISPLAY_LINES, 1);

    add_move_binding(binding_set, GDK_KEY_Up, GDK_CONTROL_MASK, GTK_MOVEMENT_PARAGRAPHS, -1);
    add_move_binding(binding_set, GDK_KEY_KP_Up, GDK_CONTROL_MASK, GTK_MOVEMENT_PARAGRAPHS, -1);

    add_move_binding(binding_set, GDK_KEY_Down, GDK_CONTROL_MASK, GTK_MOVEMENT_PARAGRAPHS, 1);
    add_move_binding(binding_set, GDK_KEY_KP_Down, GDK_CONTROL_MASK, GTK_MOVEMENT_PARAGRAPHS, 1);

    add_move_binding(binding_set, GDK_KEY_Home, 0, GTK_MOVEMENT_DISPLAY_LINE_ENDS, -1);
    add_move_binding(binding_set, GDK_KEY_KP_Home, 0, GTK_MOVEMENT_DISPLAY_LINE_ENDS, -1);

    add_move_binding(binding_set, GDK_KEY_End, 0, GTK_MOVEMENT_DISPLAY_LINE_ENDS, 1);
    add_move_binding(binding_set, GDK_KEY_KP_End, 0, GTK_MOVEMENT_DISPLAY_LINE_ENDS, 1);

    add_move_binding(binding_set, GDK_KEY_Home, GDK_CONTROL_MASK, GTK_MOVEMENT_BUFFER_ENDS, -1);
    add_move_binding(binding_set, GDK_KEY_KP_Home, GDK_CONTROL_MASK, GTK_MOVEMENT_BUFFER_ENDS, -1);

    add_move_binding(binding_set, GDK_KEY_End, GDK_CONTROL_MASK, GTK_MOVEMENT_BUFFER_ENDS, 1);
    add_move_binding(binding_set, GDK_KEY_KP_End, GDK_CONTROL_MASK, GTK_MOVEMENT_BUFFER_ENDS, 1);

    add_move_binding(binding_set, GDK_KEY_Page_Up, 0, GTK_MOVEMENT_PAGES, -1);
    add_move_binding(binding_set, GDK_KEY_KP_Page_Up, 0, GTK_MOVEMENT_PAGES, -1);

    add_move_binding(binding_set, GDK_KEY_Page_Down, 0, GTK_MOVEMENT_PAGES, 1);
    add_move_binding(binding_set, GDK_KEY_KP_Page_Down, 0, GTK_MOVEMENT_PAGES, 1);

    add_move_binding(binding_set, GDK_KEY_Page_Up, GDK_CONTROL_MASK, GTK_MOVEMENT_HORIZONTAL_PAGES, -1);
    add_move_binding(binding_set, GDK_KEY_KP_Page_Up, GDK_CONTROL_MASK, GTK_MOVEMENT_HORIZONTAL_PAGES, -1);

    add_move_binding(binding_set, GDK_KEY_Page_Down, GDK_CONTROL_MASK, GTK_MOVEMENT_HORIZONTAL_PAGES, 1);
    add_move_binding(binding_set, GDK_KEY_KP_Page_Down, GDK_CONTROL_MASK, GTK_MOVEMENT_HORIZONTAL_PAGES, 1);

    /* Select all */
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_a, GDK_CONTROL_MASK, "select-all", 0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_slash, GDK_CONTROL_MASK, "select-all", 0);

    /* Deleting text */
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Delete, (GdkModifierType)0, "delete-from-cursor", 2, G_TYPE_INT,
                                 GTK_DELETE_CHARS, G_TYPE_INT, 1);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_KP_Delete, (GdkModifierType)0, "delete-from-cursor", 2,
                                 G_TYPE_INT, GTK_DELETE_CHARS, G_TYPE_INT, 1);

    gtk_binding_entry_add_signal(binding_set, GDK_KEY_BackSpace, (GdkModifierType)0, "backspace", 0);

    /* Make this do the same as Backspace, to help with mis-typing */
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_BackSpace, GDK_SHIFT_MASK, "backspace", 0);

    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Delete, GDK_CONTROL_MASK, "delete-from-cursor", 2, G_TYPE_INT,
                                 GTK_DELETE_WORD_ENDS, G_TYPE_INT, 1);

    gtk_binding_entry_add_signal(binding_set, GDK_KEY_KP_Delete, GDK_CONTROL_MASK, "delete-from-cursor", 2, G_TYPE_INT,
                                 GTK_DELETE_WORD_ENDS, G_TYPE_INT, 1);

    gtk_binding_entry_add_signal(binding_set, GDK_KEY_BackSpace, GDK_CONTROL_MASK, "delete-from-cursor", 2, G_TYPE_INT,
                                 GTK_DELETE_WORD_ENDS, G_TYPE_INT, -1);

    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Delete, (GdkModifierType)(GDK_SHIFT_MASK | GDK_CONTROL_MASK),
                                 "delete-from-cursor", 2, G_TYPE_INT, GTK_DELETE_PARAGRAPH_ENDS, G_TYPE_INT, 1);

    gtk_binding_entry_add_signal(binding_set, GDK_KEY_KP_Delete, (GdkModifierType)(GDK_SHIFT_MASK | GDK_CONTROL_MASK),
                                 "delete-from-cursor", 2, G_TYPE_INT, GTK_DELETE_PARAGRAPH_ENDS, G_TYPE_INT, 1);

    gtk_binding_entry_add_signal(binding_set, GDK_KEY_BackSpace, (GdkModifierType)(GDK_SHIFT_MASK | GDK_CONTROL_MASK),
                                 "delete-from-cursor", 2, G_TYPE_INT, GTK_DELETE_PARAGRAPH_ENDS, G_TYPE_INT, -1);

    /* Cut/copy/paste */

    gtk_binding_entry_add_signal(binding_set, GDK_KEY_x, GDK_CONTROL_MASK, "cut-clipboard", 0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_c, GDK_CONTROL_MASK, "copy-clipboard", 0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_v, GDK_CONTROL_MASK, "paste-clipboard", 0);

    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Delete, GDK_SHIFT_MASK, "cut-clipboard", 0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Insert, GDK_CONTROL_MASK, "copy-clipboard", 0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Insert, GDK_SHIFT_MASK, "paste-clipboard", 0);

    /* Overwrite */
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Insert, (GdkModifierType)0, "toggle-overwrite", 0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_KP_Insert, (GdkModifierType)0, "toggle-overwrite", 0);
}

static void gtk_xoj_int_txt_init(GtkXojIntTxt* invisible) { gtk_widget_set_has_window(GTK_WIDGET(invisible), true); }

GtkWidget* gtk_xoj_int_txt_new(TextEditor* te) {
    GtkXojIntTxt* txt = (GtkXojIntTxt*)g_object_new(GTK_TYPE_XOJ_INT_TXT, nullptr);
    txt->te = te;
    return (GtkWidget*)txt;
}

static void gtk_invisible_realize(GtkWidget* widget) {
    gtk_widget_set_realized(widget, true);

    GdkWindow* parent = gtk_widget_get_parent_window(widget);
    if (parent == nullptr) {
        parent = gdk_screen_get_root_window(gdk_screen_get_default());
    }

    GdkWindowAttr attributes;
    attributes.x = -100;
    attributes.y = -100;
    attributes.width = 0;
    attributes.height = 0;
    attributes.window_type = GDK_WINDOW_TEMP;
    attributes.wclass = GDK_INPUT_ONLY;
    attributes.override_redirect = true;
    attributes.event_mask = gtk_widget_get_events(widget);

    gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_NOREDIR;

    gtk_widget_set_window(widget, gdk_window_new(parent, &attributes, attributes_mask));

    gdk_window_set_user_data(gtk_widget_get_window(widget), widget);
}

static void gtk_invisible_style_set(GtkWidget* widget, GtkStyle* previous_style) {
    /* Don't chain up to parent implementation */
}

static void gtk_invisible_show(GtkWidget* widget) {
    gtk_widget_set_visible(widget, true);
    gtk_widget_map(widget);
}

static void gtk_invisible_size_allocate(GtkWidget* widget, GtkAllocation* allocation) {
    gtk_widget_set_allocation(widget, allocation);
}

static GObject* gtk_invisible_constructor(GType type, guint n_construct_properties,
                                          GObjectConstructParam* construct_params) {
    GObject* object;

    object = G_OBJECT_CLASS(gtk_xoj_int_txt_parent_class)->constructor(type, n_construct_properties, construct_params);

    GtkWidget* widget = GTK_WIDGET(object);
    gtk_invisible_realize(widget);

    return object;
}
