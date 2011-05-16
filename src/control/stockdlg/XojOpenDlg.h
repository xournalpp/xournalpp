/*
 * Xournal++
 *
 * GTK Open dialog to select XOJ (or PDF) file with preview
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XOJOPENDLG_H__
#define __XOJOPENDLG_H__

#include <String.h>
#include <gtk/gtk.h>

#include "../settings/Settings.h"

class XojOpenDlg {
private:
	XojOpenDlg();
	virtual ~XojOpenDlg();

public:
	static String showOpenDialog(GtkWindow * win, Settings * settings, bool pdf, bool & attachPdf);

private:
	static void updatePreviewCallback(GtkFileChooser * fileChooser, void * userData);
};

#endif /* __XOJOPENDLG_H__ */
