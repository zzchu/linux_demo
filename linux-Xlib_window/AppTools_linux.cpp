
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory.h>
#include <iostream>


#include "AppTools_linux.h"
using namespace XTAppManager;

bool xerror = false;

XErrorEvent g_XTErrorEventStru;


int handleError(Display* display, XErrorEvent* error){
    char buffer_return[1024] = {'\0'};
    XGetErrorText(display, error->error_code, buffer_return, 1024);
    printf("X11 Error content: %s \n", buffer_return);
    memcpy((void*)&g_XTErrorEventStru, (void*)error, sizeof(XErrorEvent));
    return 1;
}

XWindowUtils::XWindowUtils()
{
// for XmbTextPropertyToTextList
//    setlocale(LC_ALL, ""); // see man locale
    _display = openDisplay();
    XSetErrorHandler(handleError);
}

XWindowUtils::~XWindowUtils()
{
    XCloseDisplay(_display);
}

Window XWindowUtils::getOpenAppWindow(long m_dwProcessId)
{
    // Get the root window for the current display.
    Window winRoot = XDefaultRootWindow(_display);
    
    WindowsMatchingPid wmp(_display,winRoot,m_dwProcessId);
    list<Window> lw = wmp.result();
    
    for(list<Window>::iterator it=lw.begin(); it != lw.end(); it++ ){
        Window topWindow = getTopmostWindow(*it);
        Window nameWin = getNamedWindow(topWindow);
        if (nameWin == topWindow) { //can't get a valid named window
            continue;
        }
        if (true == isWindowFullScreen(nameWin) && (true == isWindowVisuable(nameWin)))//is it full screen?
        {
            
            return nameWin;
        }
    }
    
    return INVALID_WINDOW_ID;
}

Window XWindowUtils::getActiveWindow()
{
    // Get the root window for the current display.
    Window activeWindow = getFocusWindow();
    Window topWindow = getTopmostWindow(activeWindow);
    return getNamedWindow(topWindow);
}

void XWindowUtils::closeWindow(Window w){
//    w = getTopmostWindow(w);
    
//    XRaiseWindow(_display, w);
//    XMapRaised(_display, w);
//    XCirculateSubwindowsUp(_display, getTopmostWindow(w));
//    printf("test before:");
//    getTestWindow();
//    getFocusWindow();
#if 0
    XWindowAttributes attr;
    Atom atom;
    XEvent xev;
    atom = XInternAtom (_display, "_NET_ACTIVE_WINDOW", False);
    
    // try sending an event, so that metacity picks up what has been set on X before.. (does not seem to work)
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.display = _display;
    xev.xclient.window = w;
    xev.xclient.message_type = atom;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 2;
    xev.xclient.data.l[1] = 0;
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = 0;
    
    XGetWindowAttributes(_display, w, &attr);
    //attr.override_redirect = (gboolean)1;
    //XChangeWindowAttributes(d, w, 0, attr);
    
    XSendEvent (_display,
                attr.root, False,
                SubstructureRedirectMask | SubstructureNotifyMask,
                &xev);
    
    XFlush(_display);
    XSync(_display, False);
    
#endif
//    printf("zz focus window: %d with type:%d \n", getFocusWindow(), getWindowType(getFocusWindow()));
//    w = getTopmostWindow(w);
//    sleep(2);
    printf("start to delete the window\n");
    XEvent ev;

    memset(&ev, 0, sizeof (ev));
    
    ev.xclient.type = ClientMessage;
    ev.xclient.window = w;
    ev.xclient.message_type = XInternAtom(_display, "WM_PROTOCOLS", true);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = XInternAtom(_display, "WM_DELETE_WINDOW", false);
    ev.xclient.data.l[1] = CurrentTime;
    
    XSetInputFocus(_display, getTopmostWindow(w), RevertToParent, CurrentTime);
    
    int count = 100;
    while (count > 0) {
        count --;
        if (getFocusWindow() != w)
            break;
        printf("zzzzz!!!!!!!!!!");
        usleep(500);
    }
    
//    sleep(2);
//    printf("test after:");
//    getTestWindow();
//    getFocusWindow();
    
    XSendEvent(_display, w, False, NoEventMask, &ev);
    XSync(_display, 0);
}


bool XWindowUtils::isWindowVisuable(Window win)
{
    XWindowAttributes xwAttr;
    
    Status ret = XGetWindowAttributes(_display, win, &xwAttr );
    if (xwAttr.map_state == IsViewable) {
        return true;
    }
    return false;
}

bool XWindowUtils::isWindowFullScreen(Window win)
{
    int w, h, screen_w, screen_h;
    
    getWindowSize(win, &w, &h);
    getScreenSize(&screen_w, &screen_h);
    //    printf (" screen Window:  width = %d, height = %d \n", w, h);
    if ((w == screen_w) && (h == screen_h))
    {
        return true;
    }
    
    return false;
}

bool XWindowUtils::isWindowMaximized(Window win)
{

    Atom wmState = XInternAtom(_display, "_NET_WM_STATE", True);
    Atom max_horz  =  XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_HORZ", true);
    Atom max_vert  =  XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_VERT", true);
    Atom type;
    int format;
    unsigned long nItem, bytesAfter;
    unsigned char *properties = NULL;
    XGetWindowProperty(_display, win, wmState, 0, (~0L), False, XA_ATOM, &type, &format, &nItem, &bytesAfter, &properties);
    
    int isMaxCount = 0;
    
    for (unsigned iItem = 0; iItem < nItem; ++iItem){
//        printf("property=%s\n",XGetAtomName(_display, ((Atom*)properties)[iItem]));
        if ((((Atom*)properties)[iItem] == max_horz) ||
            (((Atom*)properties)[iItem] == max_vert))
        {
            isMaxCount ++;
        }
    }
    XFree(properties);
    
    if (isMaxCount == 2) {
        return true;
    }
    return false;

}


XTWindowType XWindowUtils::getWindowType(Window w)
{
    Atom wmWindowType = XInternAtom(_display, "_NET_WM_WINDOW_TYPE", True);
    Atom wmWindowTypeDesktop = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_DESKTOP", True);
    Atom wmWindowTypeNormal = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_NORMAL", True);
    Atom wmWindowTypeDialog = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_DIALOG", True);
    Atom wmWindowTypeTooltip = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_TOOLTIP", True);
    Atom wmWindowTypeCombo = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_COMBO", True);
    
    
    Atom type, prop;
    int format;
    unsigned long nItem, bytesAfter;
    unsigned char *properties = NULL;
    
    int status = XGetWindowProperty(_display, w, wmWindowType, 0L, sizeof(Atom), False, XA_ATOM, &type, &format, &nItem, &bytesAfter, &properties);
    
    if (status == Success && properties) {
        
        prop = ((Atom*)properties)[0];
        if ( prop == wmWindowTypeDesktop )
        {
            return XTWindowTypeDesktop;
        }
        else if ( prop == wmWindowTypeNormal )
        {
            return XTWindowTypeNormal;
        }
        else if ( prop == wmWindowTypeDialog )
        {
            return XTWindowTypeDialog;
        }
        else if ( prop == wmWindowTypeTooltip )
        {
            return XTWindowTypeTooltip;
        }
        else if ( prop == wmWindowTypeCombo )
        {
            return XTWindowTypeCombo;
        }
        else
        {
            return XTWindowTypeOthers;
        }
    }

    
    return XTWindowTypeNone;
}

void XWindowUtils::setWindowFullScreen(Window w)
{
    Atom wm_state   = XInternAtom (_display, "_NET_WM_STATE", true );
    Atom wm_fullscreen = XInternAtom (_display, "_NET_WM_STATE_FULLSCREEN", true );
    
    XChangeProperty(_display, w, wm_state, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
}

void XWindowUtils::setWindowMaximum(Window w)
{
    XEvent xev;
    Atom wm_state  =  XInternAtom(_display, "_NET_WM_STATE", true);
    Atom max_horz  =  XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_HORZ", true);
    Atom max_vert  =  XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_VERT", true);
    
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = w;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = max_horz;
    xev.xclient.data.l[2] = max_vert;
    
    XSendEvent(_display, DefaultRootWindow(_display), False, SubstructureNotifyMask, &xev);
    XSync(_display, 0);
    
}

Display* XWindowUtils::openDisplay(){
    printf("connecting X server ... ");
    Display* d = XOpenDisplay(NULL);
    if(d == NULL){
        printf("fail\n");
        return 0;
    }else{
        printf("success\n");
    }
    return d;
}

Window XWindowUtils::getFocusWindow(){
    Window w;
    int revert_to;
    printf("getting input focus window ... ");
    XGetInputFocus(_display, &w, &revert_to); // see man
    if(xerror){
        printf("fail\n");
        return INVALID_WINDOW_ID;
    }else if(w == None){
        printf("no focus window\n");
        return INVALID_WINDOW_ID;
    }else{
        printf("success (window: %d)\n", (int)w);
    }
    
    return w;
}

Window XWindowUtils::getTestWindow(){
    
    Window root;
    Atom netactivewindow, real;
    
    int format;
    unsigned long extra, n, w;
    unsigned char *data;
    
    root = XDefaultRootWindow(_display);
    netactivewindow = XInternAtom(_display, "_NET_ACTIVE_WINDOW", False);
    
    if(XGetWindowProperty(_display, root, netactivewindow, 0, ~0, False,
                          AnyPropertyType, &real, &format, &n, &extra,
                          &data) != Success && data != 0)
    {
        return INVALID_WINDOW_ID;
    }
    
    w = *(unsigned long *) data;
    XFree (data);
    
    printf("get active window:%d \n", w);
    
    return w;
}


// get the top window.
// a top window have the following specifications.
//  * the start window is contained the descendent windows.
//  * the parent window is the root window.
Window XWindowUtils::getTopmostWindow(Window start){
    Window w = start;
    Window parent = start;
    Window root = None;
    Window *children;
    unsigned int nchildren;
    Status s;
    
    printf("getting top window ... \n");
    while (parent != root) {
        w = parent;
        s = XQueryTree(_display, w, &root, &parent, &children, &nchildren); // see man
        
        if (s)
            XFree(children);
        
        if(xerror){
            printf("fail\n");
            return INVALID_WINDOW_ID;
        }
        
        printf("  get parent (window: %d)\n", (int)w);
    }
    
    printf("success (window: %d)\n", (int)w);
    
    return w;
}

// search a named window (that has a WM_STATE prop)
// on the descendent windows of the argment Window.
Window XWindowUtils::getNamedWindow(Window start){
    Window w;
    printf("getting named window ... ");
    w = XmuClientWindow(_display, start); // see man
    printf("success (window: %d)\n", (int) w);
    return w;
}


unsigned long XWindowUtils::getWindowPid(Window win)
{
    // Get the PID property atom.
    unsigned long  pid;
    Atom _atomPID = XInternAtom(_display, "_NET_WM_PID", True);
    if(_atomPID == None)
    {
        cout << "No such atom" << endl;
        return INVALID_PID;
    }
    // Get the PID for the current Window.
    Atom           type;
    int            format;
    unsigned long  nItems;
    unsigned long  bytesAfter;
    unsigned char *propPID = 0;
    if(Success == XGetWindowProperty(_display, win, _atomPID, 0, 1, False, XA_CARDINAL,
                                     &type, &format, &nItems, &bytesAfter, &propPID))
    {
        if(propPID != 0){
            pid = *((unsigned long *)propPID);
            XFree(propPID);
            return pid;
        }
    }
    
    return INVALID_PID;
}

void XWindowUtils::getWindowSize(Window win, int *w, int *h)
{
    XWindowAttributes xwAttr;
    
    Status ret = XGetWindowAttributes(_display, win, &xwAttr );
    *w = xwAttr.width;
    *h = xwAttr.height;
}


void XWindowUtils::getScreenSize(int *w, int*h)
{
    Screen* pscr = NULL;
    
    pscr = DefaultScreenOfDisplay(_display);
    if ( !pscr ) {
        fprintf(stderr, "Failed to obtain the default screen of given display.\n");
        return;
    }
    *w = pscr->width;
    *h = pscr->height;
}
#if 0
// GET THE ACTUAL TEXT!!! - This is where it all comes unstuck
void XWindowUtils::GetWindowProperties (Display *display, Window window)
{
    Atom *atoms;
    int i, j;
    Atom type;
    int format, result, status;
    unsigned long len, bytesLeft;
    unsigned char *data;
    char *atomName;
    XTextProperty textData;
    
    atoms = XListProperties (display, window, &i);
    if (i)
    {
        for (j=0; j < i; j++)
        {
            atomName = XGetAtomName(display, atoms[i]);
            if (atomName)
                printf ("Atom name: %s/r/n", atomName);
            
            status = XGetTextProperty (display, window, &textData, atoms[i]);
            if (status)
            {
                printf ("Atom text: %s/r/n", textData.value);
            }
        }
    }
}
// END GET THE ACTUAL TEXT!!! - This is where it all comes unstuck
#endif
// ENUMARATE THROUGH WINDOWS AND DISPLAY THEIR TITLES
void XWindowUtils::EnumerateWindows (Display *display, Window rootWindow, int showErrors, int showStatus)
{
    static int level = 0;
    Window parent;
    Window *children;
    Window *child;
    unsigned int noOfChildren;
    int status;
    int i;
    
    XTextProperty wmName;
    char **list;
    
    status = XGetWMName (display, rootWindow, &wmName);
    if ((status) && (wmName.value) && (wmName.nitems))
    {
        status = XmbTextPropertyToTextList (display, &wmName, &list, &i);
        if ((status >= Success) && (i) && (*list))
        {
            if (getWindowPid(rootWindow) == 6284 ) {
                
                printf ("INFO - Found window (type:%d, pid:%ld ==>:%d)with name '%s'  \r\n", getWindowType(rootWindow),getWindowPid(rootWindow),rootWindow,(char*) strdup (*list));
                Window topWindow = getTopmostWindow(rootWindow);
                getNamedWindow(topWindow);
//                closeWindow(topWindow);
                
                
            }
        }
    }
    
    //GetWindowProperties (display, rootWindow);
    
    level++;
    
    status = XQueryTree (display, rootWindow, &rootWindow, &parent, &children, &noOfChildren);
    
    if (status == 0)
    {
        if (showErrors)
            printf ("ERROR - Could not query the window tree. Aborting.\r\n");
        return;
    }
    
    if (noOfChildren == 0)
    {
        if (showErrors)
            printf ("ERROR - No children found. Aborting.\r\n");
        return;
    }
    else
    {
        if (showStatus)
            printf ("STATUS - %i number of child windows found.\r\n", noOfChildren);
    }
    
    for (i=0; i < noOfChildren; i++)
    {
        EnumerateWindows (display, children[i], showErrors, showStatus);
    }
    
    XFree ((char*) children);
}



// (XFetchName cannot get a name with multi-byte chars)
void XWindowUtils::printWindowName(Window w){
    XTextProperty prop;
    Status s;
    
    printf("window name:\n");
    
    s = XGetWMName(_display, w, &prop); // see man
    if(!xerror && s){
        int count = 0, result;
        char **list = NULL;
        result = XmbTextPropertyToTextList(_display, &prop, &list, &count); // see man
        if(result == Success){
            printf("\t%s\n", list[0]);
        }else{
            printf("ERROR: XmbTextPropertyToTextList\n");
        }
    }else{
        printf("ERROR: XGetWMName\n");
    }
}

void XWindowUtils::printWindowClass(Window w){
    Status s;
    XClassHint* xClass;
    
    printf("application: \n");
    
    xClass = XAllocClassHint(); // see man
    if(xerror){
        printf("ERROR: XAllocClassHint\n");
    }
    
    s = XGetClassHint(_display, w, xClass); // see man
    if(xerror || s){
        printf("\tname: %s\n\txClass: %s\n", xClass->res_name, xClass->res_class);
    }else{
        printf("ERROR: XGetClassHint\n");
    }
}

void XWindowUtils::printWindowInfo(Window w){
    printf("--\n");
    printWindowName(w);
    printWindowClass(w);
}



bool XWindowUtils::isWindowExist(Window win)
{
    XWindowAttributes xwAttr;
    
    g_XTErrorEventStru.resourceid = 0;
    Status ret = XGetWindowAttributes(_display, win, &xwAttr );
    if (ret == 0){
        if ((g_XTErrorEventStru.resourceid == win) &&
            (g_XTErrorEventStru.error_code == BadWindow))
        {
            return false;
        }
    }
    return true;
}

