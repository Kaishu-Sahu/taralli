#include "common.h"
#include <time.h>

/* The main program.  Tracks the position of the X pointer, and warps it at
   screen edges, according to the function 'map' defined in another file. */

int main() {
    // Initialize X and the screen edge map.
    Display *dpy = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(dpy);
    map_init(dpy);
    
    // Get the XInput opcode.
    // (Variables starting with an underscore are not used.)
    int xi_opcode, _first_event, _first_error;
    if (! XQueryExtension(dpy, "XInputExtension", &xi_opcode,
                          &_first_event, &_first_error)) {
       puts("XInput is not available.");
       exit(1);
    }

    // Tell XInput to send us all RawMotion events.
    // (Normal Motion events are blocked by some windows.)
    unsigned char mask_bytes[XIMaskLen(XI_RawMotion)];
    memset(mask_bytes, 0, sizeof(mask_bytes));
    XISetMask(mask_bytes, XI_RawMotion);

    XIEventMask mask;
    mask.deviceid = XIAllMasterDevices;
    mask.mask_len = sizeof(mask_bytes);
    mask.mask = mask_bytes;
    XISelectEvents(dpy, root, &mask, 1);

    // Receive X events forever.
    while (1) {
	// puts("while loop iterated");
        XEvent event;

        //This is blocking which prevent the loop to run forever
        XNextEvent(dpy, &event);
        if ((event.xcookie.type == GenericEvent) &&
            (event.xcookie.extension == xi_opcode) &&
            XGetEventData(dpy, &event.xcookie)) {

            // On each RawMotion event, retrieve the pointer location
            // and move the pointer if necessary.
            if (event.xcookie.evtype == XI_RawMotion) {
                int old_x, old_y, new_x, new_y;
                int _win_x, _win_y;
                Window _root, _child;
                unsigned int _mask;
                if (XQueryPointer(dpy, root, &_root, &_child, &old_x, &old_y,
                                  &_win_x, &_win_y, &_mask)) {
                                
                    new_x = old_x;
                    new_y = old_y;
                    map(&new_x, &new_y);
                    
                    // if (_mask == ShiftMask) {
			// puts("shift pressed"); 
		    // }
		    // printf("%d\n", _mask);
		    // printf("%d\n", ShiftMask);				

                    if ((_mask == 1 || _mask == 17) && ((new_x != old_x) || (new_y != old_y)) ) {
                        XWarpPointer(dpy, None, root, 0, 0, 0, 0,
                                     new_x, new_y);
                    }
                }
            }

            // Clean up after XGetEventData.
            XFreeEventData(dpy, &event.xcookie);
        }
    }
}
