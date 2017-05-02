#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "X11CursorShapeGrabber.h"

namespace XTCursorShapeGrabber {
    
    X11CursorShapeGrabber::X11CursorShapeGrabber()
    :have_xfixes_(false),
    xfixes_event_base_(-1),
    xfixes_error_base_(-1),
    cursor_change_flag_(false){
        x_display_ = SharedXDisplay::CreateDefault();
        window_ = DefaultRootWindow(x_display_->display());
        cursorImage_ = NULL;
        Init();
    }
    
    X11CursorShapeGrabber::~X11CursorShapeGrabber() {
        if (cursorImage_ != NULL) {
            XFree(cursorImage_);
            cursorImage_ = NULL;
        }

        if (have_xfixes_) {
            x_display_->RemoveEventHandler(xfixes_event_base_ + XFixesCursorNotify,
                                           this);
        }
        if (x_display_ != NULL) {
            delete x_display_;
            x_display_ = NULL;
        }
    }
    
    void X11CursorShapeGrabber::Init() {
        // Init can be called only once per instance of MouseCursorMonitor.
        have_xfixes_ = XFixesQueryExtension(display(), &xfixes_event_base_, &xfixes_error_base_);
        
        if (have_xfixes_) {
            // Register for changes to the cursor shape.
            XFixesSelectCursorInput(display(), window_, XFixesDisplayCursorNotifyMask);
            x_display_->AddEventHandler(xfixes_event_base_ + XFixesCursorNotify, this);
            
            CaptureCursor();
        } else {
//            LOG(LS_INFO) << "X server does not support XFixes.";
            printf("X server does not support XFixes.\n");
        }
    }
    
    bool X11CursorShapeGrabber::isCursorShapeChanged()
    {
        // Process X11 events in case XFixes has sent cursor notification.
        x_display_->ProcessPendingXEvents();
        
        return cursor_change_flag_;
    }
    
    bool X11CursorShapeGrabber::grab() {
        
        if (cursor_change_flag_) {
            cursor_change_flag_ = false;
        }
        else
        {
            return false;
        }
        
        CaptureCursor();
        return true;
        
    }
    
    const int X11CursorShapeGrabber::getWidth() const{
        return cursorImage_->width;
    }
    const int X11CursorShapeGrabber::getHeight() const   {
        return cursorImage_->height;
    }

    const void* X11CursorShapeGrabber::getBuffer()  const  {
        return cursorImage_->pixels;
    }
    const char* X11CursorShapeGrabber::getMask()  const  {
        return 0;
    }
    const int X11CursorShapeGrabber::getXHotSpot()  const  {
        return cursorImage_->xhot;
    }
    const int X11CursorShapeGrabber::getYHotSpot() const   {
        return cursorImage_->yhot;
    }
    
    bool X11CursorShapeGrabber::HandleXEvent(const XEvent& event) {
        if (have_xfixes_ && event.type == xfixes_event_base_ + XFixesCursorNotify) {
            const XFixesCursorNotifyEvent* cursor_event =
            reinterpret_cast<const XFixesCursorNotifyEvent*>(&event);
            if (cursor_event->subtype == XFixesDisplayCursorNotify) {
                cursor_change_flag_ = true;
            }
            // Return false, even if the event has been handled, because there might be
            // other listeners for cursor notifications.
        }
        return false;
    }
    
    void X11CursorShapeGrabber::CaptureCursor() {
        assert(have_xfixes_);
//        XFixesCursorImage* img;
        if (cursorImage_ != NULL) {
            XFree(cursorImage_);
        }
        cursorImage_ = XFixesGetCursorImage(display());
//        width_ = cursorImage_->width;
//        height_ = cursorImage_->height;
//        xhot_ = cursorImage_->xhot;
//        yhot_ = cursorImage_->yhot;
//        pixels_ = cursorImage_->pixels;
//        XFree(cursorImage_);
    }
}
