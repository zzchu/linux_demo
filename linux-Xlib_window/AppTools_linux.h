#pragma once
#include <stdio.h>
#include <list>
#include <X11/Xlib.h>           // `apt-get install libx11-dev`
#include <X11/Xmu/WinUtil.h>    // `apt-get install libxmu-dev`
#include <X11/Xatom.h>
using namespace std;

#define INVALID_WINDOW_ID     ((unsigned long)~0x0)
#define INVALID_PID           ((unsigned long)~0x0)


typedef enum
{
    XTWindowTypeNone = 0,
    XTWindowTypeDesktop = 1,
    XTWindowTypeNormal = 2,
    XTWindowTypeDialog = 3,
    XTWindowTypeTooltip = 4,
    XTWindowTypeCombo = 5,
    XTWindowTypeOthers = 6
} XTWindowType;


namespace XTAppManager
{
    
    class WindowsMatchingPid{
    public:
        WindowsMatchingPid(Display *display, Window wRoot, unsigned long pid)
        : _display(display)
        , _pid(pid)
        {
            // Get the PID property atom.
            _atomPID = XInternAtom(display, "_NET_WM_PID", True);
            if(_atomPID == None){
                printf("No such atom.\n");
                return;
            }
            
            search(wRoot);
        }
        
        static unsigned long getWindowsPid(Display *display, Window win);
        
        const list<Window> &result() const { return _result; }
        
    private:
        unsigned long  _pid;
        Atom           _atomPID;
        Display       *_display;
        list<Window>   _result;
        
        void search(Window w)
        {
            // Get the PID for the current Window.
            Atom           type;
            int            format;
            unsigned long  nItems;
            unsigned long  bytesAfter;
            unsigned char *propPID = 0;
            if(Success == XGetWindowProperty(_display, w, _atomPID, 0, 1, False, XA_CARDINAL,
                                             &type, &format, &nItems, &bytesAfter, &propPID)){
                if(propPID != 0){
                    // If the PID matches, add this window to the result set.
                    if(_pid == *((unsigned long *)propPID))
                        _result.push_back(w);
                    
                    XFree(propPID);
                }
            }
            
            // Recurse into child windows.
            Window    wRoot;
            Window    wParent;
            Window   *wChild;
            unsigned  nChildren;
            if(0 != XQueryTree(_display, w, &wRoot, &wParent, &wChild, &nChildren)){
                for(unsigned i = 0; i < nChildren; i++)
                    search(wChild[i]);
            }
        }
        
    };
    
    class XWindowUtils
    {
    public:
        XWindowUtils();
        ~XWindowUtils();
        
        Window getOpenAppWindow(long m_dwProcessId);
        Window getActiveWindow();
        bool isWindowFullScreen(Window win);
        bool isWindowMaximized(Window win);
        bool isWindowExist(Window win);
        void setWindowMaximum(Window w);
        bool isWindowVisuable(Window win);
        void setWindowFullScreen(Window w);
        void closeWindow(Window w);
        void printWindowInfo(Window w);
        void EnumerateWindows (Display *display, Window rootWindow, int showErrors, int showStatus);
        XTWindowType getWindowType(Window w);
    private:
        Display* _display;
        list<Window>   _result;

        Display* openDisplay();
        Window getFocusWindow();
        Window getTestWindow();
        Window getTopmostWindow(Window start);
        Window getNamedWindow(Window start);
        unsigned long getWindowPid(Window win);
        void getWindowSize(Window win, int *w, int *h);
        void getScreenSize(int *w, int *h);
        void GetWindowProperties (Display *display, Window window);
        void printWindowName(Window w);
        void printWindowClass(Window w);
    };
}

