// -*- coding:utf-8-unix; mode:c; -*-
/*
 get the active window on X window system
 http://k-ui.jp/blog/2012/05/07/get-active-window-on-x-window-system/
 http://stackoverflow.com/questions/16720961/many-xsetinputfocuss-and-xsync-causes-error
 https://john.nachtimwald.com/2009/11/08/sending-wm_delete_window-client-messages/
 http://stackoverflow.com/questions/1157364/intercept-wm-delete-window-on-x11
 */
#include <unistd.h>
#include "AppTools_linux.h"

using namespace XTAppManager;

#if 0
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory.h>
#include <iostream>
#include <list>

#include <X11/Xlib.h>           // `apt-get install libx11-dev`
#include <X11/Xmu/WinUtil.h>    // `apt-get install libxmu-dev`
#include <X11/Xatom.h>

using namespace std;

Bool xerror = False;

Display* open_display(){
    printf("connecting X server ... ");
    Display* d = XOpenDisplay(NULL);
    if(d == NULL){
        printf("fail\n");
        exit(1);
    }else{
        printf("success\n");
    }
    return d;
}

int handle_error(Display* display, XErrorEvent* error){
    printf("ERROR: X11 error\n");
    xerror = True;
    return 1;
}

Window get_focus_window(Display* d){
    Window w;
    int revert_to;
    printf("getting input focus window ... ");
    XGetInputFocus(d, &w, &revert_to); // see man
    if(xerror){
        printf("fail\n");
        exit(1);
    }else if(w == None){
        printf("no focus window\n");
        exit(1);
    }else{
        printf("success (window: %d)\n", (int)w);
    }
    
    return w;
}

// get the top window.
// a top window have the following specifications.
//  * the start window is contained the descendent windows.
//  * the parent window is the root window.
Window get_top_window(Display* d, Window start){
    Window w = start;
    Window parent = start;
    Window root = None;
    Window *children;
    unsigned int nchildren;
    Status s;
    
    printf("getting top window ... \n");
    while (parent != root) {
        w = parent;
        s = XQueryTree(d, w, &root, &parent, &children, &nchildren); // see man
        
        if (s)
            XFree(children);
        
        if(xerror){
            printf("fail\n");
            exit(1);
        }
        
        printf("  get parent (window: %d)\n", (int)w);
    }
    
    printf("success (window: %d)\n", (int)w);
    
    return w;
}

// search a named window (that has a WM_STATE prop)
// on the descendent windows of the argment Window.
Window get_named_window(Display* d, Window start){
    Window w;
    printf("getting named window ... ");
    w = XmuClientWindow(d, start); // see man
    if(w == start)
        printf("fail\n");
    printf("success (window: %d)\n", (int) w);
    return w;
}

// (XFetchName cannot get a name with multi-byte chars)
void print_window_name(Display* d, Window w){
    XTextProperty prop;
    Status s;
    
    printf("window name:\n");
    
    s = XGetWMName(d, w, &prop); // see man
    if(!xerror && s){
        int count = 0, result;
        char **list = NULL;
        result = XmbTextPropertyToTextList(d, &prop, &list, &count); // see man
        if(result == Success){
            printf("\t%s\n", list[0]);
        }else{
            printf("ERROR: XmbTextPropertyToTextList\n");
        }
    }else{
        printf("ERROR: XGetWMName\n");
    }
}

void print_window_class(Display* d, Window w){
    Status s;
    XClassHint* xClass;
    
    printf("application: \n");
    
    xClass = XAllocClassHint(); // see man
    if(xerror){
        printf("ERROR: XAllocClassHint\n");
    }
    
    s = XGetClassHint(d, w, xClass); // see man
    if(xerror || s){
        printf("\tname: %s\n\txClass: %s\n", xClass->res_name, xClass->res_class);
    }else{
        printf("ERROR: XGetClassHint\n");
    }
}

void print_window_info(Display* d, Window w){
    printf("--\n");
    print_window_name(d, w);
    print_window_class(d, w);
}

void send_close_to_window(Display* d, Window w){
    printf("start to delete the window\n");
    XEvent ev;
    
    memset(&ev, 0, sizeof (ev));
    
    ev.xclient.type = ClientMessage;
    ev.xclient.window = w;
    ev.xclient.message_type = XInternAtom(d, "WM_PROTOCOLS", true);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = XInternAtom(d, "WM_DELETE_WINDOW", false);
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(d, w, False, NoEventMask, &ev);
}


unsigned long get_window_pid(Display *display, Window win)
{
    // Get the PID property atom.
    unsigned long  _pid;
    Atom _atomPID = XInternAtom(display, "_NET_WM_PID", True);
    if(_atomPID == None)
    {
        cout << "No such atom" << endl;
        return 0;
    }
    // Get the PID for the current Window.
    Atom           type;
    int            format;
    unsigned long  nItems;
    unsigned long  bytesAfter;
    unsigned char *propPID = 0;
    if(Success == XGetWindowProperty(display, win, _atomPID, 0, 1, False, XA_CARDINAL,
                                     &type, &format, &nItems, &bytesAfter, &propPID))
    {
        if(propPID != 0)
        {
            _pid = *((unsigned long *)propPID);
            XFree(propPID);
            return _pid;
        }
    }
    
    return 0;
}

int getWindowSize(Display *display, Window win, int *w, int *h)
{
//    Display* pdsp = NULL;
//    Window wid = 0;
    XWindowAttributes xwAttr;
    
//    pdsp = XOpenDisplay( NULL );
//    if ( !pdsp ) {
//        fprintf(stderr, "Failed to open default display.\n");
//        return -1;
//    }
    
//    wid = DefaultRootWindow( pdsp );
//    if ( 0 > wid ) {
//        fprintf(stderr, "Failed to obtain the root windows Id "
//                "of the default screen of given display.\n");
//        return -2;
//    }
    
    Status ret = XGetWindowAttributes( display, win, &xwAttr );
    *w = xwAttr.width;
    *h = xwAttr.height;
    printf (" window:  x = %d, y = %d \n", xwAttr.x, xwAttr.y);
//    XCloseDisplay( pdsp );
    return 0;
}

bool isWindowVisuable(Display *display, Window win)
{
    XWindowAttributes xwAttr;
    
    Status ret = XGetWindowAttributes( display, win, &xwAttr );
    if (xwAttr.map_state == IsViewable) {
        return true;
    }
    return false;
}

int getScreenSize(int *w, int*h)
{
    
    Display* pdsp = NULL;
    Screen* pscr = NULL;
    
    pdsp = XOpenDisplay( NULL );
    if ( !pdsp ) {
        fprintf(stderr, "Failed to open default display.\n");
        return -1;
    }
    
    pscr = DefaultScreenOfDisplay( pdsp );
    if ( !pscr ) {
        fprintf(stderr, "Failed to obtain the default screen of given display.\n");
        return -2;
    }
    
    *w = pscr->width;
    *h = pscr->height;
    
    XCloseDisplay( pdsp );
    return 0;
}
#endif

#if 0
class WindowsMatchingPid{
public:
    WindowsMatchingPid(Display *display, Window wRoot, unsigned long pid)
    : _display(display)
    , _pid(pid)
    {
        // Get the PID property atom.
        _atomPID = XInternAtom(display, "_NET_WM_PID", True);
        if(_atomPID == None)
        {
            cout << "No such atom" << endl;
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
                                         &type, &format, &nItems, &bytesAfter, &propPID))
        {
            if(propPID != 0)
            {
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
        if(0 != XQueryTree(_display, w, &wRoot, &wParent, &wChild, &nChildren))
        {
            for(unsigned i = 0; i < nChildren; i++)
                search(wChild[i]);
        }
    }
    
};
#endif

int main(void){
    XWindowUtils xWindowUtils = XWindowUtils();
    sleep(2);

    //Window win = xWindowUtils.getOpenAppWindow(5941);
    //xWindowUtils.printWindowInfo(win);
    
    Window openWindow = xWindowUtils.getActiveWindow();
    if (INVALID_WINDOW_ID == openWindow) {
        printf("get the opened window failed!!!!\n");
    }
    printf("the window type: %d\n", xWindowUtils.getWindowType(openWindow));
    
//    sleep(2);
    if (false == xWindowUtils.isWindowMaximized(openWindow))
    {
        printf ("window is not max!\n");
        xWindowUtils.setWindowMaximum(openWindow);
    }
    
    printf("start testing\n");
    if (false == xWindowUtils.isWindowExist(73400435)) {
        printf("window isnot exist!\n");
    }
    
    if (false == xWindowUtils.isWindowExist(1234)) {
        printf("windowxxx exist!\n");
    }
//
//    openWindow = xWindowUtils.getActiveWindow();
//    if (INVALID_WINDOW_ID == openWindow) {
//        printf("get the opened window failed!!!!\n");
//    }
//    
//    xWindowUtils.printWindowInfo(openWindow);
    
//    xWindowUtils.closeWindow(openWindow);
//
//    sleep(1);
//    
//    if (true == xWindowUtils.isWindowFullScreen(openWindow))
//    {
//        printf("window is full screen\n");
//    }
//    if (true == xWindowUtils.isWindowVisuable(openWindow))
//    {
//        printf("window is viewable\n");
//    }
//    else
//    {
//            printf("window is disappear\n");
//    }
    
#if 0
    Display *display = XOpenDisplay (NULL);
    int screen = DefaultScreen (display);
    Window rootWindow = RootWindow (display, screen);
    xWindowUtils.EnumerateWindows (display, rootWindow, 0, 0);
    XCloseDisplay (display);
#endif
    printf("end!!!\n");
}
