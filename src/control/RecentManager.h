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
// TODO: AA: type check

#ifndef __RECENTMANAGER_H__
#define __RECENTMANAGER_H__

#include <gtk/gtk.h>
#include "../util/String.h"

class RecentManagerListener {
public:
	virtual void fileOpened(const char * uri) = 0;
};

class RecentManager {
public:
	RecentManager();
	virtual ~RecentManager();

public:
	void addRecentFileFilename(const char * filename);
	void addRecentFileUri(const char * uri);
	void removeRecentFileFilename(const char * filename);
	void removeRecentFileUri(const char * uri);

	void freeOldMenus();
	void updateMenu();

	int getMaxRecent();
	void setMaxRecent(int maxRecent);

	void openRecent(String uri);

	GtkWidget * getMenu();

	void addListener(RecentManagerListener * listener);

private:
	GList * filterRecent(GList * items, bool xoj);
	void addRecentMenu(GtkRecentInfo * info, int i);

	static void recentManagerChangedCallback(GtkRecentManager * manager, RecentManager * recentManager);
	static void recentsMenuActivateCallback(GtkAction * action, RecentManager * recentManager);
	static int sortRecentsEntries(GtkRecentInfo * a, GtkRecentInfo * b);

private:
	int maxRecent;
	int recentHandlerId;

	GList * listener;

	GtkWidget * menu;
	GList * menuItemList;
};

#endif /* __RECENTMANAGER_H__ */
