/*
 * Xournal++
 *
 * GTK Open dialog to select XOJ (or PDF) file with preview
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __XOJOPENDLG_H__
#define __XOJOPENDLG_H__

#include <StringUtils.h>
#include <gtk/gtk.h>

#include "../settings/Settings.h"

class XojOpenDlg
{
private:
	XojOpenDlg();
	virtual ~XojOpenDlg();

public:
	static path showOpenDialog(GtkWindow* win, Settings* settings, bool pdf,
							bool& attachPdf);

private:
	static void updatePreviewCallback(GtkFileChooser* fileChooser, void* userData);
};

#endif /* __XOJOPENDLG_H__ */
