
#include <algorithm>
#include <stdio.h>
#include "XDisplayUtils.h"

namespace XTCursorShapeGrabber {
    
    SharedXDisplay::SharedXDisplay(Display* display)
    : display_(display) {
        assert(display_);
    }
    
    SharedXDisplay::~SharedXDisplay() {
        assert(event_handlers_.empty());
        XCloseDisplay(display_);
    }
    
    // static
    SharedXDisplay* SharedXDisplay::Create(const std::string& display_name) {
        Display* display =
        XOpenDisplay(display_name.empty() ? NULL : display_name.c_str());
        if (!display) {
            printf("Unable to open display \n");
            return NULL;
        }
        return new SharedXDisplay(display);
    }
    
    // static
    SharedXDisplay* SharedXDisplay::CreateDefault() {
        return Create(std::string());
    }
    
    void SharedXDisplay::AddEventHandler(int type, XEventHandler* handler) {
        event_handlers_[type].push_back(handler);
    }
    
    void SharedXDisplay::RemoveEventHandler(int type, XEventHandler* handler) {
        EventHandlersMap::iterator handlers = event_handlers_.find(type);
        if (handlers == event_handlers_.end())
            return;
        
        std::vector<XEventHandler*>::iterator new_end =
        std::remove(handlers->second.begin(), handlers->second.end(), handler);
        handlers->second.erase(new_end, handlers->second.end());
        
        // Check if no handlers left for this event.
        if (handlers->second.empty())
            event_handlers_.erase(handlers);
    }
    
    void SharedXDisplay::ProcessPendingXEvents() {
        // Hold reference to |this| to prevent it from being destroyed while
        // processing events.

        // Find the number of events that are outstanding "now."  We don't just loop
        // on XPending because we want to guarantee this terminates.
        int events_to_process = XPending(display());
        XEvent e;
        
        for (int i = 0; i < events_to_process; i++) {
            XNextEvent(display(), &e);
            EventHandlersMap::iterator handlers = event_handlers_.find(e.type);
            if (handlers == event_handlers_.end())
                continue;
            for (std::vector<XEventHandler*>::iterator it = handlers->second.begin();
                 it != handlers->second.end(); ++it) {
                if ((*it)->HandleXEvent(e))
                    break;
            }
        }
    }
    
}
