#include<stdio.h>
#include<unistd.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

// The key code to be sent.
// A full list of available codes can be found in /usr/include/X11/keysymdef.h
#define KEYCODE XK_W

// Function to create a keyboard event
XKeyEvent createKeyEvent(Display *display, Window &win,
                         Window &winRoot, bool press,
                         int keycode, int modifiers)
{
    XKeyEvent event;
    
    event.display     = display;
    event.window      = win;
    event.root        = winRoot;
    event.subwindow   = None;
    event.time        = CurrentTime;
    event.x           = 1;
    event.y           = 1;
    event.x_root      = 1;
    event.y_root      = 1;
    event.same_screen = True;
    event.keycode     = XKeysymToKeycode(display, keycode);
    event.state       = modifiers;
    
    if(press)
        event.type = KeyPress;
    else
        event.type = KeyRelease;
    
    return event;
}

void mouseClick(Display *display, int button)
{
    //    Display *display = XOpenDisplay(NULL);
    
    XEvent event;
    
    if(display == NULL)
    {
        fprintf(stderr, "Errore nell'apertura del Display !!!\n");
        exit(EXIT_FAILURE);
    }
    
    memset(&event, 0x00, sizeof(event));
    
    printf("start to send button %d\n", button);
    
    event.type = ButtonPress;
    event.xbutton.button = button;
    event.xbutton.same_screen = True;
    
    XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    
    event.xbutton.subwindow = event.xbutton.window;
    
    printf("x=>%d, y=>%d, state=>%d\n", event.xbutton.x, event.xbutton.y, event.xbutton.state);
    
    while(event.xbutton.subwindow)
    {
        event.xbutton.window = event.xbutton.subwindow;
        
        XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    }
    
    printf("x=>%d, y=>%d\n", event.xbutton.x, event.xbutton.y);
    
    if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");
    
    XFlush(display);
    
    usleep(100000);
    
    event.type = ButtonRelease;
    event.xbutton.state = Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask;
    
    if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");
    
    XFlush(display);
    
    //    XCloseDisplay(display);
}

main(int argc,char * argv[])
{
    int i=0;
    int x , y;
    x=atoi(argv[1]);
    y=atoi(argv[2]);

    // Obtain the X11 display.
    Display *display = XOpenDisplay(0);
    if(display == NULL)
        return -1;
    
    // Get the root window for the current display.
    Window winRoot = XDefaultRootWindow(display);
    
    printf("x=> %d, y=> %d\n", x, y);
    XWarpPointer(display, None, winRoot, 0, 0, 0, 0, x, y);
    XFlush(display);
    
    //button1 left button
    //
    mouseClick(display, Button3);
#if 0
    // Find the window which has the current keyboard focus.
    Window winFocus;
    int    revert;
    XGetInputFocus(display, &winFocus, &revert);
    
    // Send a fake key press event to the window.
    XKeyEvent event = createKeyEvent(display, winFocus, winRoot, true, KEYCODE, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
    
    // Send a fake key release event to the window.
    event = createKeyEvent(display, winFocus, winRoot, false, KEYCODE, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
#endif
    // Done.
    XCloseDisplay(display);

    return 0;
}
