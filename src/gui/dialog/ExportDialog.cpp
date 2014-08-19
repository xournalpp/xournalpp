#include "ExportDialog.h"
#include <PageRange.h>

#include <glib/gi18n-lib.h>

ExportDialog::ExportDialog(GladeSearchpath* gladeSearchPath, Settings* settings,
                           int pageCount, int currentPage) :
	GladeGui(gladeSearchPath, "export.glade", "exportDialog")
{

	XOJ_INIT_TYPE(ExportDialog);

	this->range = NULL;
	this->pageCount = pageCount;
	this->currentPage = currentPage;
	this->resolution = 72;
	this->settings = settings;
	this->type = EXPORT_FORMAT_PNG;
	this->typesModel = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
	this->typesView = GTK_TREE_VIEW(get("lstTypes"));

	setupModel();

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spPngResolution")),
	                          300);

	g_signal_connect(get("txtPages"), "focus-in-event",
	                 G_CALLBACK(&rangeFocused),
	                 (gpointer) this);

	g_signal_connect(G_OBJECT(this->typesView), "cursor-changed",
	                 G_CALLBACK(&fileTypeSelected),
	                 (gpointer) this);

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(this->window),
	                                    settings->getLastSavePath().c_str());

	g_signal_connect(this->window, "selection-changed",
	                 G_CALLBACK(&selectionChanged),
	                 (gpointer) this);

}

void ExportDialog::setupModel()
{
	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();

	gtk_tree_view_insert_column_with_attributes(this->typesView,
	                                            -1,
	                                            _("File Type"),
	                                            renderer,
	                                            "text", 0,
	                                            NULL);



	gtk_tree_view_insert_column_with_attributes(this->typesView,
	                                            -1,
	                                            _("Extension"),
	                                            renderer,
	                                            "text", 1,
	                                            NULL);

	GtkTreeViewColumn* col = gtk_tree_view_get_column(this->typesView,
	                                                  COL_FILEDESC);

	gtk_tree_view_column_set_expand(col, TRUE);

	col = gtk_tree_view_get_column(this->typesView,
	                               COL_EXTENSION);

	gtk_tree_view_column_set_expand(col, FALSE);

	//use alternating colors
	g_object_set(G_OBJECT(this->typesView), "rules-hint", TRUE, NULL);

	gtk_tree_view_set_model(this->typesView,
	                        GTK_TREE_MODEL(this->typesModel));

	addFileType(_("By extension"), NULL, 0, "All files", true);
	addFileType("Portable Document Format",  "pdf", EXPORT_FORMAT_PDF);
	addFileType("Portable Network Graphics", "png", EXPORT_FORMAT_PNG);
	addFileType("Scalable Vector Graphics",  "svg", EXPORT_FORMAT_SVG);
	addFileType("Encapsulated PostScript",   "eps", EXPORT_FORMAT_EPS);
}

void ExportDialog::addFileType(const char* typeDesc,
                               const char* pattern,
                               gint type,
                               const char* filterName,
                               bool select)
{
	GtkFileFilter *filter = gtk_file_filter_new();
	String fullName;

	if(pattern)
	{
		fullName = String::format("%s (*.%s)",
		                          filterName ? filterName : typeDesc,
		                          pattern);
		gtk_file_filter_set_name(filter, fullName.c_str());
		gtk_file_filter_add_pattern(filter, pattern);
	}
	else
	{
		gtk_file_filter_set_name(filter,
		                         filterName ? filterName : typeDesc);
		gtk_file_filter_add_pattern(filter, "*");
	}

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(this->window),
	                            filter);

	GtkTreeIter iter;
	gtk_list_store_append(this->typesModel, &iter);

	gtk_list_store_set(this->typesModel, &iter,
	                   COL_FILEDESC, typeDesc,
	                   COL_EXTENSION, pattern,
	                   COL_TYPE, type,
	                   -1);

	if(select)
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(this->typesView),
		                               &iter);
}

ExportDialog::~ExportDialog()
{
	XOJ_RELEASE_TYPE(ExportDialog);

	g_object_unref(this->typesModel);
}

GList* ExportDialog::getRange()
{
	XOJ_CHECK_TYPE(ExportDialog);

	return this->range;
}

void ExportDialog::selectionChanged(GtkFileChooser* chooser,
                                    gpointer user_data)
{
	ExportDialog* dlg = (ExportDialog*) user_data;

	char* file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->window));

	gtk_widget_set_sensitive(GTK_WIDGET(dlg->get("btnExport")), !!file);

	if(file)
		g_free(file);
}

void ExportDialog::handleData()
{
	XOJ_CHECK_TYPE(ExportDialog);

	//	GtkWidget * rdRangeAll = get("rdRangeAll");
	GtkWidget* rdRangeCurrent = get("rdRangeCurrent");
	GtkWidget* rdRangePages = get("rdRangePages");

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangePages)))
	{
		this->range = PageRange::parse(gtk_entry_get_text(GTK_ENTRY(get("txtPages"))));
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangeCurrent)))
	{
		this->range = g_list_append(this->range,
		                            new PageRangeEntry(this->currentPage,
		                                               this->currentPage));
	}
	else
	{
		this->range = g_list_append(this->range, new PageRangeEntry(0,
		                                                            this->pageCount - 1));
	}

	this->resolution = gtk_spin_button_get_value(GTK_SPIN_BUTTON(
	                                                 get("spPngResolution")));

	this->settings->setLastSavePath(this->getFolder());
}

ExportFormtType ExportDialog::getFormatType()
{
	XOJ_CHECK_TYPE(ExportDialog);

	return this->type;
}

int ExportDialog::getPngDpi()
{
	XOJ_CHECK_TYPE(ExportDialog);

	return this->resolution;
}

gboolean ExportDialog::rangeFocused(GtkWidget* widget,
                                    GdkEvent* event,
                                    gpointer user_data)
{
	ExportDialog* dlg = (ExportDialog*) user_data;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->get("rdRangePages")),
	                             TRUE);

	// the event still needs to be processed...
	return FALSE;
}

void ExportDialog::fileTypeSelected(GtkTreeView* treeview,
                                    gpointer user_data)
{
	ExportDialog* dlg = (ExportDialog*) user_data;

	GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dlg->window));

	if(file)
	{
		GtkTreeSelection* selection = gtk_tree_view_get_selection(dlg->typesView);
		GtkTreeIter iter;

		if(gtk_tree_selection_get_selected(selection,
		                                   NULL,
		                                   &iter))
		{
			gchar* extension;

			gtk_tree_model_get(GTK_TREE_MODEL(dlg->typesModel),
			                   &iter, COL_EXTENSION, &extension, -1);

			if(extension)
			{
				char* _baseName = g_file_get_basename(file);
				String baseName = _baseName, newName;
				bool changeName = true;

				g_free(_baseName);

				gint _type;

				gtk_tree_model_get(GTK_TREE_MODEL(dlg->typesModel),
				                   &iter, COL_TYPE, &_type, -1);

				dlg->type = (ExportFormtType) _type;

				String s = baseName;
				int nameIndex = baseName.lastIndexOf(String("."));

				if(nameIndex == -1)
				{
					newName = String::format("%s.%s", baseName.c_str(), extension);
				}
				else
				{
					if(String(extension) != baseName.substring(nameIndex))
					{
						newName = String::format("%s.%s",
						                         baseName.substring(0, nameIndex).c_str(),
						                         extension);
					}
					else
					{
						changeName = false;
					}
				}
				
				if(changeName)
				{
					gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dlg->window),
					                                  newName.c_str());
				}
			}
		}
	}
}

String ExportDialog::getFolder()
{
	XOJ_CHECK_TYPE(ExportDialog);

	String s;
	GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(this->window));

	if(file)
	{
		gchar* folderName = g_file_get_path(file);

		s = folderName;

		g_free(folderName);
	}

	int index = s.lastIndexOf(G_DIR_SEPARATOR_S);

	if(index != -1)
	{
		s = s.substring(0, index);
	}

	return s;
}

String ExportDialog::getFilename()
{
	XOJ_CHECK_TYPE(ExportDialog);

	String s;
	GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(this->window));

	if(file)
	{
		gchar* fileName = g_file_get_basename(file);

		s = fileName;

		g_free(fileName);
	}

	return s;
}

bool ExportDialog::validate()
{
	XOJ_CHECK_TYPE(ExportDialog);

	if(validExtension())
	{
		// now we know that we have a valid file, lets just try
		// to determine the file type

		return validFilename();
	}

	return false;
}

bool ExportDialog::validFilename()
{
	XOJ_CHECK_TYPE(ExportDialog);

	GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(this->window));

	if(!file)
	{
		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *this,
		                                           GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
		                                           _("Invalid filename selected"));

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return false;
	}

	if(g_file_query_exists(file, NULL))
	{
		char *baseName = g_file_get_basename(file);
		char *_dirName = g_file_get_path(file);
		
		String dirName = _dirName;
		
		int firstIndex = 0, secIndex = 0, tmp, len = dirName.length();
		
		while(secIndex < len &&
		      -1 != (tmp = dirName.indexOf(G_DIR_SEPARATOR_S, secIndex + 1)))
		{
			firstIndex = secIndex;
			secIndex = tmp;
		}

		if(secIndex != 0 && secIndex > firstIndex + 1)
		{
			dirName = dirName.substring(firstIndex + 1,
			                            secIndex - firstIndex - 1);
		}

		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *this,
		                                           GTK_DIALOG_DESTROY_WITH_PARENT,
		                                           GTK_MESSAGE_WARNING,
		                                           GTK_BUTTONS_OK_CANCEL,
		                                           _("A file named \"%s\" already exists. Do you want to replace it?"),
		                                           baseName);

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
		                                         _("The file already exists in \"%s\". Replacing it will overwrite its contents."),
		                                         dirName.c_str());

		g_free(baseName);
		g_free(_dirName);

		gint result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		
		if(result != GTK_RESPONSE_OK)
			return false;
	}

	return true;
}

bool ExportDialog::validExtension()
{
	XOJ_CHECK_TYPE(ExportDialog);

	GtkTreeSelection* selection = gtk_tree_view_get_selection(this->typesView);
	GtkTreeIter iter;
	bool validExtension = false;

	if(gtk_tree_selection_get_selected(selection,
	                                   NULL,
	                                   &iter))
	{
		gtk_tree_model_get(GTK_TREE_MODEL(this->typesModel),
		                   &iter, COL_TYPE, &this->type, -1);

		if(this->type)
		{
			validExtension = true;
		}
		else
		{
			validExtension = fileTypeByExtension();
		}
	}
	else
	{
		validExtension = fileTypeByExtension();
	}

	if(!validExtension)
	{
		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *this,
		                                           GTK_DIALOG_DESTROY_WITH_PARENT,
		                                           GTK_MESSAGE_WARNING,
		                                           GTK_BUTTONS_OK,
		                                           _("The given filename does not have any known "
		                                             "file extension. Please enter a known file "
		                                             "extension or select a file format "
		                                             "from the file format list."));

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return false;
	}

	return true;
}

bool ExportDialog::fileTypeByExtension()
{
	XOJ_CHECK_TYPE(ExportDialog);

	GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(this->window));
	
	if(!file)
		return false;
	
	char *_baseName = g_file_get_basename(file);
	String baseName = _baseName;
	g_free(_baseName);
	int index = baseName.lastIndexOf(".");
	bool validExtension = false;
	GtkTreeIter iter;

	if(index != -1)
	{
		String extension = baseName.substring(index + 1);
		
		gboolean validIterator =
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(this->typesModel), &iter);
		
		while(validIterator)
		{
			gchar* currentExtension;
			
			gtk_tree_model_get(GTK_TREE_MODEL(this->typesModel), &iter,
			                   COL_EXTENSION, &currentExtension,
			                   -1);

			if(extension == currentExtension)
			{
				gtk_tree_model_get(GTK_TREE_MODEL(this->typesModel),
				                   &iter, COL_TYPE, &this->type, -1);
				
				validExtension = true;
				g_free(currentExtension);
				break;
			}

			g_free(currentExtension);

			validIterator = gtk_tree_model_iter_next (GTK_TREE_MODEL(this->typesModel), &iter);
		}
	}

	return validExtension;
}

void ExportDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(ExportDialog);

	int res = 0;
	do
	{
		gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
		res = gtk_dialog_run(GTK_DIALOG(this->window));

		if (res == 2)
		{
			if (validate())
			{
				handleData();
				break;
			}
		}
	}
	while (res == 2);

	gtk_widget_hide(this->window);
}

