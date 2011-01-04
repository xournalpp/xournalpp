/*
 * Xournal++
 *
 * Extended GLIB acces to underlying structures, needed for Xournal
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#ifndef __POPPLERGLIBEXTENSION_H__
#define __POPPLERGLIBEXTENSION_H__

#include <poppler/TextOutputDev.h>
#include <gtk/gtk.h>
#include <poppler.h>

TextPage * poppler_page_get_text_page(PopplerPage * page);
GList *poppler_page_find_text_xoj(PopplerPage *page, const char *text);



#endif /* __POPPLERGLIBEXTENSION_H__ */
