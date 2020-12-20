#include "AbstractInputDevice.h"

#include "control/Control.h"
#include "control/settings/ButtonConfig.h"
#include "gui/Layout.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "gui/widgets/XournalWidget.h"


AbstractInputDevice::AbstractInputDevice(GtkWidget* widget, XournalView* view): widget(widget), view(view) {}

AbstractInputDevice::~AbstractInputDevice() {
    widget = nullptr;
    view = nullptr;
}
