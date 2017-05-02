
#pragma once

#include <map>
#include <vector>
#include <assert.h>
#include <X11/Xlib.h>
#include <string>

namespace XTCursorShapeGrabber {
    
    // A ref-counted object to store XDisplay connection.
    class SharedXDisplay {
    public:
        class XEventHandler {
        public:
            virtual ~XEventHandler() {}
            
            // Processes XEvent. Returns true if the event has been handled.
            virtual bool HandleXEvent(const XEvent& event) = 0;
        };
        
        // Takes ownership of |display|.
        explicit SharedXDisplay(Display* display);
        ~SharedXDisplay();
        // Creates a new X11 Display for the |display_name|. NULL is returned if X11
        // connection failed. Equivalent to CreateDefault() when |display_name| is
        // empty.
        static SharedXDisplay* Create(
                                      const std::string& display_name);
        
        // Creates X11 Display connection for the default display (e.g. specified in
        // DISPLAY). NULL is returned if X11 connection failed.
        static SharedXDisplay* CreateDefault();
        
        //        void AddRef() { ++ref_count_; }
        //        void Release() {
        //            if (--ref_count_ == 0)
        //                delete this;
        //        }
        //
        Display* display() { return display_; }
        
        // Adds a new event |handler| for XEvent's of |type|.
        void AddEventHandler(int type, XEventHandler* handler);
        
        // Removes event |handler| added using |AddEventHandler|. Doesn't do anything
        // if |handler| is not registered.
        void RemoveEventHandler(int type, XEventHandler* handler);
        
        // Processes pending XEvents, calling corresponding event handlers.
        void ProcessPendingXEvents();
        
    private:
        typedef std::map<int, std::vector<XEventHandler*> > EventHandlersMap;
        
        
        
        //        Atomic32 ref_count_;
        Display* display_;
        
        EventHandlersMap event_handlers_;
        
        //        RTC_DISALLOW_COPY_AND_ASSIGN(SharedXDisplay);
    };
    
}

