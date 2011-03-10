/*
 * Xournal++
 *
 * Static definitions for Xournal++
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

// PREF FILES INFO
#define CONFIG_DIR          ".xournalpp"
#define TOOLBAR_CONFIG      "toolbar.ini"
#define SETTINGS_XML_FILE   "settings.xml"
#define PRINT_CONFIG_FILE   "print-config.ini"
#define METADATA_FILE 	    "xo-metadata.xml"
#define METADATA_MAX_ITEMS	50

/**
 * This should be enabled, else after screen rotation pen support does not work anymore
 */
#define ENABLE_XINPUT_BUGFIX

///////////////////////////////////////
// Enable debugging ///////////////////

// Input debugging, e.g. eraser events etc.
// #define INPUT_DEBUG

// Shape recognizer debug: output score etc.
// #define RECOGNIZER_DEBUG

// Scheduler debug: show jobs etc.
// #define SHEDULER_DEBUG

// draw a surrounding border to all elements
#define SHOW_ELEMENT_BOUNDS

// draw a border around all repaint rects
// #define SHOW_REPAINT_BOUNDS


#endif /* __CONFIG_H__ */
