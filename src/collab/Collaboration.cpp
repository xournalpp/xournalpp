#include "Collaboration.h"
#include "../control/Control.h"
#include "../model/Stroke.h"
#include "../model/Layer.h"
#include "../gui/PageView.h"
#include "../gui/XournalView.h"

Collaboration::Collaboration(Control * control) {
	this->control = control;
}

Collaboration::~Collaboration() {
}

/**
 * This is the entry point, show a dialog to configure settings, login, whatever
 *
 * first you can implement it hardcoded.
 */
void Collaboration::start() {
	printf("Collaboration::start()\n");

	// example insert an element
	Document * doc = control->getDocument();

	// you have always to lock the document before doing anything!
	doc->lock();
	// now we can access the document, from every thread, it sould be threadsave;-)

	PageRef page = doc->getPage(0);
	if(page.isValid()) { // Page 0 should always be valid, but we check this,
		Stroke * s = new Stroke();
		s->setColor(0xff0000);
		s->setWidth(1);
		s->setToolType(STROKE_TOOL_PEN);
		s->addPoint(Point(0,0)); // in document coordinates
		s->addPoint(Point(100, 100));
		page.getSelectedLayer()->addElement(s);

		// so, document editing is finished, but now we need to inform the view he needs to rerender the selection
		// MVC pattern usually do this automatically, but we cannot do this automatically, because
		// if we change a lot of things we only repaints once the whole area its much faster

		PageView * view = control->getWindow()->getXournal()->getViewFor(0);

		// this call should also be threadsave;-) Hopefully...
		view->rerenderElement(s);

		// repaint is needed if a e.g. a selection is changed, rerender if contents is changed

		// THIS IS NOT THREADSAVE, should be done in the gui thread
		page.getSelectedLayer()->addListener(this);
	}

	doc->unlock();

}


void Collaboration::layerDeletedCb() {
	printf("Layer deleted\n");
}

void Collaboration::elementAdded(Element * e) {
	printf("Element added\n");
}

void Collaboration::elementRemoved(Element * e) {
	printf("Element removed\n");
}


