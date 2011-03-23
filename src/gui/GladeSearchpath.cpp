#include "GladeSearchpath.h"
// TODO: AA: type check

GladeSearchpath::GladeSearchpath() {
	this->directories = NULL;
}

GladeSearchpath::~GladeSearchpath() {
	for (GList * l = this->directories; l != NULL; l = l->next) {
		char * str = (char *)l->data;
		g_free(str);
	}

	g_list_free(this->directories);
}

char * GladeSearchpath::findFile(const char * subdir, const char * file) {
	GList * elem;

	char * filename = NULL;
	if (subdir == NULL) {
		filename = g_strdup(file);
	} else {
		filename = g_strdup_printf("%s%c%s", subdir, G_DIR_SEPARATOR, file);
	}

	// We step through each directory to find it.
	for (GList * l = this->directories; l != NULL; l = l->next) {
		gchar * pathname = g_strdup_printf("%s%c%s", (char*) l->data, G_DIR_SEPARATOR, filename);

		if (g_file_test(pathname, G_FILE_TEST_EXISTS)) {
			g_free(filename);
			return pathname;
		}

		g_free(pathname);
	}

	g_free(filename);
	return NULL;
}

/*
 * Use this function to set the directory containing installed pixmaps and Glade XML files.
 */
void GladeSearchpath::addSearchDirectory(const char * directory) {
	this->directories = g_list_append(this->directories, g_strdup(directory));
}
