
//
// Created by ulrich on 12.04.19.
//

#include "KeyboardInputHandler.h"
#include "control/settings/PageTemplateSettings.h"  // for PageTemplateSettings

#include <gdk/gdk.h>         // for _GdkEventKey, gdk...
#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Down, GDK...

#include "control/tools/EditSelection.h"            // for EditSelection
#include "gui/XournalView.h"                        // for XournalView
#include "gui/inputdevices/AbstractInputHandler.h"  // for AbstractInputHandler
#include "gui/inputdevices/InputEvents.h"           // for GdkEventGuard
#include "gui/widgets/XournalWidget.h"              // for GtkXournal

#include "InputContext.h"  // for InputContext

KeyboardInputHandler::KeyboardInputHandler(InputContext* inputContext): AbstractInputHandler(inputContext) {}

KeyboardInputHandler::~KeyboardInputHandler() = default;

auto KeyboardInputHandler::handleImpl(InputEvent const& event) -> bool {
    
    GtkXournal* xournal = inputContext->getXournal();
    GdkEvent* gdkEvent = event.sourceEvent;
    PageTemplateSettings model;

    double width = model.getPageWidth();

    if (gdk_event_get_event_type(gdkEvent) == GDK_KEY_PRESS) {
    
        auto keyEvent = reinterpret_cast<GdkEventKey*>(gdkEvent);
        EditSelection* selection = xournal->selection;
        
        double shapeWidth = xournal->selection->getWidth();

        std::cout << "the-width: " << xournal->selection->getWidth() << "sdl";
        
        if (selection) {
            int d = 3;
            if (keyEvent->state & GDK_MOD1_MASK) {
                d = 1;
            } else if (keyEvent->state & GDK_SHIFT_MASK) {
                d = 20;
            }

            int xdir = 0;
            int ydir = 0;

            if (keyEvent->keyval == GDK_KEY_Alt_L) {
                //(((width / 2) - (shapeWidth / 2)) - this peice of code means that we place it in the middle
                // of the page, and we make sure we put the shape in the middle by dividing the width by 2;
                selection->placeSelection(((width / 2) - (shapeWidth / 2)), xournal->selection->getOriginalYOnView());
                selection->ensureWithinVisibleArea();
                return true;
            }
            
            
            //note to shaan:clean up.
          
            if (keyEvent->keyval == GDK_KEY_Left) {
                xdir = -1;
            } else if (keyEvent->keyval == GDK_KEY_Up) {
                //ydir = -1;
                
            } else if (keyEvent->keyval == GDK_KEY_Right) {
                xdir = 1;
            } else if (keyEvent->keyval == GDK_KEY_Down) {
                ydir = 1;
            }
        

            if (xdir != 0 || ydir != 0) {
                selection->moveSelection(d * xdir, d * ydir);
                selection->ensureWithinVisibleArea();
                return true;
            }
        }
        return xournal->view->onKeyPressEvent(keyEvent);
    } else if (gdk_event_get_event_type(gdkEvent) == GDK_KEY_RELEASE) {
        auto keyEvent = reinterpret_cast<GdkEventKey*>(gdkEvent);
        return inputContext->getView()->onKeyReleaseEvent(keyEvent);
    }

    return false;
}
