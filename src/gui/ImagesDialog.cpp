#include "ImagesDialog.h"
#include "Shadow.h"
#include <math.h>
#include "../gettext.h"
#include "../util/Util.h"

class ImageView {
public:
	ImageView(int id, ImagesDialog * dlg) {
		this->widget = gtk_drawing_area_new();
		gtk_widget_show(this->widget);
		this->crBuffer = NULL;
		this->selected = false;
		this->dlg = dlg;
		this->id = id;
		this->width = -1;
		this->height = -1;
		this->zoom = 1;

		gtk_widget_set_events(widget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

		g_signal_connect(this->widget, "expose_event", G_CALLBACK(exposeEventCallback), this);
		g_signal_connect(this->widget, "button-press-event", G_CALLBACK(mouseButtonPressCallback), this);
	}

	virtual ~ImageView() {
		gtk_widget_destroy(this->widget);

		if (crBuffer) {
			cairo_surface_destroy(crBuffer);
			crBuffer = NULL;
		}
	}

	GtkWidget * getWidget() {
		updateSize();
		return this->widget;
	}

	void calcSize() {
		if (this->width == -1) {
			GdkPixbuf * p = backgroundImage.getPixbuf();
			this->width = gdk_pixbuf_get_width(p);
			this->height = gdk_pixbuf_get_height(p);

			if (this->width < this->height) {
				zoom = 128.0 / this->height;
			} else {
				zoom = 128.0 / this->width;
			}
			this->width *= zoom;
			this->height *= zoom;

			printf("size: %i/%i\n", this->width, this->height);
		}
	}

	int getWidth() {
		calcSize();
		return width + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
	}

	int getHeight() {
		calcSize();
		return height + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
	}

	void setSelected(bool selected) {
		if (this->selected == selected) {
			return;
		}
		this->selected = selected;

		repaint();
	}

	void repaint() {
		if (crBuffer) {
			cairo_surface_destroy(crBuffer);
			crBuffer = NULL;
		}
		gtk_widget_queue_draw(this->widget);
	}

	void updateSize() {
		gtk_widget_set_size_request(widget, this->getWidth(), this->getHeight());
	}
private:
	static gboolean exposeEventCallback(GtkWidget *widget, GdkEventExpose *event, ImageView * page) {
		page->paint();
		return true;
	}

	static gboolean mouseButtonPressCallback(GtkWidget *widget, GdkEventButton *event, ImageView * page) {
		page->dlg->setSelected(page->id);
		return true;
	}

	void paint() {
		dlg->setBackgroundWhite();

		GtkAllocation alloc;
		gtk_widget_get_allocation(widget, &alloc);

		if (crBuffer == NULL) {
			crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);

			cairo_t * cr2 = cairo_create(crBuffer);
			cairo_matrix_t defaultMatrix = { 0 };
			cairo_get_matrix(cr2, &defaultMatrix);

			cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);

			cairo_scale(cr2, zoom, zoom);

			GdkPixbuf * p = backgroundImage.getPixbuf();
			gdk_cairo_set_source_pixbuf(cr2, p, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);
			cairo_paint(cr2);

			cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);

			cairo_set_matrix(cr2, &defaultMatrix);

			cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);

			cairo_set_source_rgb(cr2, 1, 1, 1);
			//left
			cairo_rectangle(cr2, 0, 0, Shadow::getShadowTopLeftSize() + 2, alloc.height);
			//top
			cairo_rectangle(cr2, 0, 0, alloc.width, Shadow::getShadowTopLeftSize() + 2);

			//right
			cairo_rectangle(cr2, alloc.width - Shadow::getShadowBottomRightSize() - 2, 0,
					Shadow::getShadowBottomRightSize() + 2, alloc.height);
			//bottom
			cairo_rectangle(cr2, 0, alloc.height - Shadow::getShadowBottomRightSize() - 2, alloc.width,
					Shadow::getShadowBottomRightSize() + 2);
			cairo_fill(cr2);

			cairo_set_operator(cr2, CAIRO_OPERATOR_ATOP);

			if (this->selected) {
				// Draw border
				Util::cairo_set_source_rgbi(cr2, dlg->getSettings()->getSelectionColor());
				cairo_set_line_width(cr2, 2);
				cairo_set_line_cap(cr2, CAIRO_LINE_CAP_BUTT);
				cairo_set_line_join(cr2, CAIRO_LINE_JOIN_BEVEL);

				cairo_rectangle(cr2, Shadow::getShadowTopLeftSize() + 1.5, Shadow::getShadowTopLeftSize() + 1.5, width
						+ 2, height + 2);

				cairo_stroke(cr2);

				Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize(), Shadow::getShadowTopLeftSize(), width + 4,
						height + 4, 0, 0, 0);
			} else {
				Shadow::drawShadow(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2, width,
						height, 0, 0, 0);
			}

			cairo_destroy(cr2);
		}

		cairo_t * cr = gdk_cairo_create(widget->window);

		double width = cairo_image_surface_get_width(crBuffer);
		cairo_set_source_surface(cr, crBuffer, 0, 0);

		cairo_paint(cr);
		cairo_destroy(cr);
	}

public:
	BackgroundImage backgroundImage;

private:
	bool selected;
	ImagesDialog * dlg;
	int id;
	int width;
	int height;
	double zoom;

	GtkWidget * widget;

	cairo_surface_t * crBuffer;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


ImagesDialog::ImagesDialog(Document * doc, Settings * settings) :
	GladeGui("images.glade", "ImagesDialog") {

	this->images = NULL;
	this->settings = settings;
	this->widget = gtk_layout_new(NULL, NULL);
	this->scrollPreview = gtk_scrolled_window_new(NULL, NULL);
	this->backgroundInitialized = false;
	this->selected = -1;
	this->lastWidth = -1;
	this->selectedPage = -1;

	// TODO: may find a better solution... depending on screen size or so
	gtk_widget_set_size_request(this->window, 800, 600);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollPreview), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollPreview), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER (scrollPreview), widget);
	gtk_box_pack_start(GTK_BOX(get("vbox")), scrollPreview, true, true, 0);

	g_signal_connect(this->window, "size-allocate", G_CALLBACK(sizeAllocate), this);

	int x = 0;
	for (int i = 0; i < doc->getPageCount(); i++) {
		XojPage * p = doc->getPage(i);

		if (p->getBackgroundType() == BACKGROUND_TYPE_IMAGE) {
			if (p->backgroundImage.getPixbuf() == NULL) {
				continue;
			}

			bool found = false;

			for (GList * l = this->images; l != NULL; l = l->next) {
				ImageView * v = (ImageView *) l->data;
				if (v->backgroundImage == p->backgroundImage) {
					found = true;
					break;
				}
			}
			if (!found) {
				ImageView * page = new ImageView(x, this);
				page->backgroundImage = p->backgroundImage;
				page->updateSize();
				gtk_layout_put(GTK_LAYOUT(this->widget), page->getWidget(), 0, 0);

				this->images = g_list_append(this->images, page);
				x++;
			}
		}
	}

	if (this->images != 0) {
		setSelected(0);
	}

	gtk_widget_show_all(this->scrollPreview);

	layout();
	updateOkButton();

	g_signal_connect(get("buttonOk"), "clicked", G_CALLBACK (okButtonCallback), this);
	g_signal_connect(get("btFilechooser"), "clicked", G_CALLBACK (filechooserButtonCallback), this);
}

ImagesDialog::~ImagesDialog() {
	for (GList * l = this->images; l != NULL; l = l->next) {
		delete (ImageView*) l->data;
	}
}

Settings * ImagesDialog::getSettings() {
	return this->settings;
}

void ImagesDialog::updateOkButton() {
	ImageView * p = (ImageView *) g_list_nth_data(this->images, this->selected);
	gtk_widget_set_sensitive(get("buttonOk"), p && gtk_widget_get_visible(p->getWidget()));
}

void ImagesDialog::okButtonCallback(GtkButton *button, ImagesDialog * dlg) {
	dlg->selectedPage = dlg->selected;
}

void ImagesDialog::filechooserButtonCallback(GtkButton *button, ImagesDialog * dlg) {
	dlg->selectedPage = -2;
	gtk_widget_hide(dlg->window);
}

bool ImagesDialog::shouldShowFilechooser() {
	return this->selectedPage == -2;
}

BackgroundImage * ImagesDialog::getSelectedImage() {
	ImageView * p = (ImageView *) g_list_nth_data(this->images, this->selectedPage);
	if (p == NULL) {
		return NULL;
	}
	return &p->backgroundImage;
}

void ImagesDialog::sizeAllocate(GtkWidget *widget, GtkRequisition *requisition, ImagesDialog * dlg) {
	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(dlg->scrollPreview, &alloc);
	if (dlg->lastWidth == alloc.width) {
		return;
	}
	dlg->lastWidth = alloc.width;
	dlg->layout();
}

void ImagesDialog::show() {
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
}

void ImagesDialog::setBackgroundWhite() {
	if (this->backgroundInitialized) {
		return;
	}
	this->backgroundInitialized = true;
	gdk_window_set_background(GTK_LAYOUT(this->widget)->bin_window, &this->widget->style->white);
}

void ImagesDialog::setSelected(int selected) {
	if (this->selected == selected) {
		return;
	}

	int lastSelected = this->selected;
	ImageView * p = (ImageView *) g_list_nth_data(this->images, selected);
	if (p) {
		p->setSelected(true);
		this->selected = selected;
	}
	p = (ImageView *) g_list_nth_data(this->images, lastSelected);
	if (p) {
		p->setSelected(false);
	}

	updateOkButton();
}

void ImagesDialog::layout() {
	double x = 0;
	double y = 0;
	double height = 0;
	double width = 0;

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(this->scrollPreview, &alloc);

	for (GList * l = this->images; l != NULL; l = l->next) {
		ImageView * p = (ImageView*) l->data;

		if (!gtk_widget_get_visible(p->getWidget())) {
			continue;
		}

		if (x + p->getWidth() > alloc.width) {
			width = MAX(width, x);
			y += height;
			x = 0;
			height = 0;
		}

		gtk_layout_move(GTK_LAYOUT(this->widget), p->getWidget(), x, y);

		height = MAX(height, p->getHeight());

		x += p->getWidth();
	}

	gtk_layout_set_size(GTK_LAYOUT(this->widget), width, y);

}
