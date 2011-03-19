#include "SaveJob.h"

#include <config.h>
#include <glib/gi18n-lib.h>

#include "../SaveHandler.h"
#include "../Control.h"
#include "../../view/DocumentView.h"

SaveJob::SaveJob(Control * control) :
	BlockingJob(control, _("Save")) {
}

SaveJob::~SaveJob() {
}

void SaveJob::run() {
	save();

	if (this->control->getWindow()) {
		callAfterRun();
	}
}

void SaveJob::afterRun() {
	if (!this->lastError.isEmpty()) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *control->getWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				"%s", this->lastError.c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	} else {
		Document * doc = this->control->getDocument();

		doc->lock();
		String filename = doc->getFilename();
		doc->unlock();

		control->getUndoRedoHandler()->documentSaved();
		control->getRecentManager()->addRecentFileFilename(filename.c_str());
		control->updateWindowTitle();
	}
}

void SaveJob::copyProgressCallback(goffset current_num_bytes, goffset total_num_bytes, Control * control) {
	printf("copyProgressCallback: %i, %i\n", (int) current_num_bytes, (int) total_num_bytes);
}

bool SaveJob::copyFile(String source, String target) {

	// we need to build the GFile from a path.
	// But if future versions support URIs, this has to be changed
	GFile * src = g_file_new_for_path(source.c_str());
	GFile * trg = g_file_new_for_path(target.c_str());
	GError * err = NULL;

	bool ok = g_file_copy(src, trg, G_FILE_COPY_OVERWRITE, NULL, (GFileProgressCallback) &copyProgressCallback, this, &err);

	if (!err && !ok) {
		this->copyError = "Copy error: return false, but didn't set error message";
	}
	if (err) {
		ok = false;
		this->copyError = err->message;
		g_error_free(err);
	}
	return ok;
}

void SaveJob::updatePreview() {
	const int previewSize = 128;

	Document * doc = this->control->getDocument();

	doc->lock();

	if (doc->getPageCount() > 0) {
		XojPage * page = doc->getPage(0);

		double width = page->getWidth();
		double height = page->getHeight();

		double zoom = 1;

		if (width < height) {
			zoom = previewSize / height;
		} else {
			zoom = previewSize / width;
		}
		width *= zoom;
		height *= zoom;

		cairo_surface_t * crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

		cairo_t * cr = cairo_create(crBuffer);
		cairo_scale(cr, zoom, zoom);
		XojPopplerPage * popplerPage = NULL;

		if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
			int pgNo = page->getPdfPageNr();
			popplerPage = doc->getPdfPage(pgNo);
			if (popplerPage) {
				popplerPage->render(cr, false);
			}
		}

		DocumentView view;
		view.drawPage(page, cr, true);
		cairo_destroy(cr);
		doc->setPreview(crBuffer);
		cairo_surface_destroy(crBuffer);
	} else {
		doc->setPreview(NULL);
	}

	doc->unlock();
}

bool SaveJob::save() {
	updatePreview();
	Document * doc = this->control->getDocument();

	SaveHandler h;

	doc->lock();
	h.prepareSave(doc);
	String filename = doc->getFilename();
	doc->unlock();

	if (doc->shouldCreateBackupOnSave()) {
		String backup = doc->getFilename();
		backup += ".bak";
		if (!copyFile(doc->getFilename(), backup)) {
			g_warning("Could not create backup! (The file was created from an older Xournal version)\n%s\n", this->copyError.c_str());
		}

		doc->setCreateBackupOnSave(false);
	}

	GzOutputStream * out = new GzOutputStream(filename);

	if (!out->getLastError().isEmpty()) {
		if (!control->getWindow()) {
			g_error(_("Open file error: %s"), out->getLastError().c_str());
			return false;
		}

		this->lastError = String::format(_("Open file error: %s"), out->getLastError().c_str());

		delete out;
		return false;
	}

	h.saveTo(out, filename);
	out->close();

	if (!out->getLastError().isEmpty()) {
		this->lastError = String::format(_("Open file error: %s"), out->getLastError().c_str());
		if (!control->getWindow()) {
			g_error("%s", this->lastError.c_str());
			return false;
		}

		delete out;
		return false;
	}

	delete out;

	return true;
}

