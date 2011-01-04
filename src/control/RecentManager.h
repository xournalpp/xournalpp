/*
 * Xournal++
 *
 * The recent opened files
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __RECENTMANAGER_H__
#define __RECENTMANAGER_H__

#include <gtk/gtk.h>

class RecentManagerListener {
public:
	virtual void fileOpened(const char * uri) = 0;
};

class RecentManager {
public:
	RecentManager();
	virtual ~RecentManager();

	void addRecentFile(const char * uri);
	void removeRecentFile(const char * uri);

	void freeOldMenus();
	void updateMenu();

	int getMaxRecent();
	void setMaxRecent(int maxRecent);

	void openRecent(const char * uri);

	GtkWidget * getMenu();

	void addListener(RecentManagerListener * listener);

private:
	GList * filterRecent(GList * items, bool xoj);
	void addRecentMenu(GtkRecentInfo *info, int i);

private:
	int maxRecent;
	int recentHandlerId;

	GList * listener;

	GtkWidget* menu;
	GList * menuItemList;
};

#endif /* __RECENTMANAGER_H__ */
