/*
 * Xournal++
 *
 * GTK Open dialog to select image with preview
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __IMAGEOPENDLG_H__
#define __IMAGEOPENDLG_H__

#include <gtk/gtk.h>
class Settings;

class ImageOpenDlg
{
private:
	ImageOpenDlg();
	virtual ~ImageOpenDlg();

public:
	static GFile* show(GtkWindow* win, Settings* settings, bool localOnly = false,
					bool* attach = NULL);

private:
	static void updatePreviewCallback(GtkFileChooser* fileChooser, void* userData);
	static GdkPixbuf* pixbufScaleDownIfNecessary(GdkPixbuf* pixbuf, gint maxSize);
};

#endif /* __IMAGEOPENDLG_H__ */
