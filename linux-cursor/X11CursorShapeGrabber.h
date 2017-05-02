#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include "XDisplayUtils.h"

namespace XTCursorShapeGrabber {
    
    class X11CursorShapeGrabber : public SharedXDisplay::XEventHandler
    {
    public:
        X11CursorShapeGrabber();
        virtual ~X11CursorShapeGrabber();
     
        virtual bool grab();
        virtual bool isCursorShapeChanged();
        
        const int getWidth() const;
        const int getHeight() const;
        
        const void *getBuffer() const;
        const char *getMask() const;
        
        const int getXHotSpot() const;
        const int getYHotSpot() const;
    private:
        void Init();
        // SharedXDisplay::XEventHandler interface.
        bool HandleXEvent(const XEvent& event);
        
        Display* display() { return x_display_->display(); }
        
        // Captures current cursor shape and stores it in |cursor_shape_|.
        void CaptureCursor();
        
        bool have_xfixes_;
        int xfixes_event_base_;
        int xfixes_error_base_;
        
        bool cursor_change_flag_;
        SharedXDisplay *x_display_;
        Window window_;
        XFixesCursorImage* cursorImage_;
        
    };
}
