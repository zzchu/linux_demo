#include "XTClipBoardLinux.h"
//#include "XTLog.h"
#include <stdlib.h>
#include <vector>
#include <thread>
#include <unistd.h>
#include <ctype.h>
#ifdef HAVE_ICONV
#include <errno.h>
#include <iconv.h>
#endif
#include <X11/extensions/XTest.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "base64.h"

using namespace std;
//using namespace XTUtils;
using namespace XTClipBoard;

/* xcin() contexts */
#define XCLIB_XCIN_NONE		0
#define XCLIB_XCIN_SELREQ	1
#define XCLIB_XCIN_INCR		2

#define LOG4CXX_DEBUG fprintf
#define LOG4CXX_ERROR fprintf
#define AppLogger     stdout

ClipBoardLinux::ClipBoardLinux():
m_selectionBuf(NULL),
m_selectionLen(0),
m_clipFormat(kClipFormatNone),
m_threadState(THREAD_STOPPED),
m_win(0)
{
    m_clipboardItemVect.clear();
}


ClipBoardLinux::~ClipBoardLinux(void)
{
}

unsigned int ClipBoardLinux::SetMutliClipboardData()
{
    //std::string clipData;
    //std::vector<UINT8> clipData;

    m_threadStateMutex.lock();
    
    LOG4CXX_DEBUG(AppLogger, "start new selection thread\n");
    std::thread listen_thread(&ClipBoardLinux::ListenSelectionRequest,this);
    listen_thread.detach();
    m_threadStateMutex.unlock();
    return true;
}

bool ClipBoardLinux::SetClipboardData(unsigned int clipFormat, std::string strData_base64)
{
    //std::string clipData;
    std::vector<UINT8> clipData;
    
    LOG4CXX_DEBUG(AppLogger, "set m_threadState THREAD_STOPPING\n");
    //check thread state, stop last selection request listen thread
    if(m_threadState == THREAD_RUNNING)
    {
        m_threadState = THREAD_STOPPING;
        LOG4CXX_DEBUG(AppLogger, "send wakeup event");
        Display *dpy = XOpenDisplay (NULL);
        XClientMessageEvent dummyEvent;
        memset(&dummyEvent, 0, sizeof(XClientMessageEvent));
        dummyEvent.type = ClientMessage;
        dummyEvent.window = m_win;
        dummyEvent.format = 32;
        XSendEvent(dpy, m_win, 0, 0, (XEvent*)&dummyEvent);
        XFlush(dpy);
        XCloseDisplay (dpy);
    }
    
    base64_decodebytes(strData_base64, clipData);
    
    m_selectionLen = clipData.size();
    
    m_selectionBuf = (unsigned char*)malloc(m_selectionLen * sizeof(char));
    
    m_clipFormat = (ClipFormat)clipFormat;
    
    if(m_selectionBuf == NULL)
        return false;
    
    memcpy(m_selectionBuf, clipData.data(), m_selectionLen);
    
    m_threadStateMutex.lock();
    
    LOG4CXX_DEBUG(AppLogger, "start new selection thread\n");
    std::thread listen_thread(&ClipBoardLinux::ListenSelectionRequest,this);
    listen_thread.detach();
    m_threadStateMutex.unlock();
    return true;
}

void ClipBoardLinux::SendCtrlV()
{
    Display *dpy = XOpenDisplay (NULL);
    
    //Wait for the running of the clipboard processing thread
    signed int count = 20;
    while(m_threadState != THREAD_RUNNING){
        usleep(50);
        count --;
        if (count <= 0) break;
    }
    
    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), true, 0);
    XSync(dpy, 0);
    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_v), true, 0);
    XSync(dpy, 0);
    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_v), false, 0);
    XSync(dpy, 0);
    XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), false, 0);
    XSync(dpy, 0);
    XCloseDisplay (dpy);
    
}

unsigned int ClipBoardLinux::GetJFIFid()
{
    return kClipFormatJfif;
}

unsigned int ClipBoardLinux::GetPNGid()
{
    return kClipFormatPng;
}

unsigned int ClipBoardLinux::GetGIFid()
{
    return kClipFormatGif;
}

unsigned int ClipBoardLinux::GetTextid()
{
    return kClipFormatText;
}

unsigned int ClipBoardLinux::GetDibid()
{
    return kClipFormatDib;
}

std::string ClipBoardLinux::JPG2DIB(std::string jpg_base64)
{
    return jpg_base64;
}

std::string ClipBoardLinux::TextTranscoding(std::string text_base64)
{
    return text_base64;
}
#if 1
unsigned ClipBoardLinux::ListenSelectionRequest()
{
    //XEvent evt;			/* X Event Structures */
    Display *dpy;
    Atom sseln;
    Atom target;
    
    /* Connect to the X server. */
    if (!(dpy = XOpenDisplay(NULL))) {
        char *display = getenv("DISPLAY");
        printf("failed!!!! display\n");
        //LOG4CXX_ERROR(AppLogger, string("Error: Can't open display: %s")<< display);
    }
    
    /* X selection to work with */
    sseln = XA_CLIPBOARD(dpy);

    /* Create a window to trap events */
    m_win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 0, 0, 0);
    
    /* get events about property changes */
    XSelectInput(dpy, m_win, PropertyChangeMask);
    
    /* take control of the selection so that we receive
     ** SelectionRequest events from other windows
     **/
    /* FIXME: Should not use CurrentTime, according to ICCCM section 2.1 */
    XSetSelectionOwner(dpy, sseln, m_win, CurrentTime);
    
    LOG4CXX_DEBUG(AppLogger, "ListenSelectionRequest running\n");
    m_threadStateMutex.lock();
    /*init thread start state*/
    m_threadState = THREAD_RUNNING;
    /* wait for a SelectionRequest event */
    while (m_threadState == THREAD_RUNNING) {
        static unsigned int clear = 0;
        static unsigned int context = XCLIB_XCIN_NONE;
        static unsigned long sel_pos = 0;
        static Window cwin;
        static Atom pty;
        int finished;
        
        XEvent evt = {0};
        
        XNextEvent(dpy, &evt);
        
        finished = xcin(dpy, &cwin, evt, &pty, &sel_pos, &context);
        
        if (evt.type == SelectionClear)
            clear = 1;
        
        if ((context == XCLIB_XCIN_NONE) && clear)
            break;
        
        if (m_threadState == THREAD_STOPPING)
            break;
    }

    m_selectionLen = 0;
    if(m_selectionBuf)
    {
        delete m_selectionBuf;
        m_selectionBuf = NULL;
    }
    m_threadState = THREAD_STOPPED;
    m_threadStateMutex.unlock();
    LOG4CXX_DEBUG(AppLogger, "ListenSelectionRequest stopped\n");
    return EXIT_SUCCESS;
}

#else
unsigned ClipBoardLinux::ListenSelectionRequest()
{
    unsigned char *sel_buf = (unsigned char*)m_selectionBuf;	/* buffer for selection data */
    unsigned long sel_len = m_selectionLen;	/* length of sel_buf */
    //XEvent evt;			/* X Event Structures */
    Display *dpy;
    Atom sseln;
    Atom target;
    
    /* Connect to the X server. */
    if (!(dpy = XOpenDisplay(NULL))) {
        char *display = getenv("DISPLAY");
        printf("failed!!!! display\n");
        //LOG4CXX_ERROR(AppLogger, string("Error: Can't open display: %s")<< display);
    }
    
    /* X selection to work with */
    sseln = XA_CLIPBOARD(dpy);
    target = XA_UTF8_STRING(dpy); //default value is string
    if(m_clipFormat == kClipFormatPng) {
        target = XInternAtom(dpy, "image/png", False);
    }
    else if(m_clipFormat == kClipFormatHtml) {
        target = XInternAtom(dpy, "text/html", False);
    }
    else if(m_clipFormat == kClipFormatRtf) {
        target = XInternAtom(dpy, "text/rtf", False);
    }
    else if(m_clipFormat == kClipFormatBmp) {
        target = XInternAtom(dpy, "image/bmp", False);
    }
    else if(m_clipFormat == kClipFormatGif) {
        target = XInternAtom(dpy, "image/gif", False);
    }
    
    /* Create a window to trap events */
    m_win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 0, 0, 0);
    
    /* get events about property changes */
    XSelectInput(dpy, m_win, PropertyChangeMask);
    
    /* take control of the selection so that we receive
     ** SelectionRequest events from other windows
     **/
    /* FIXME: Should not use CurrentTime, according to ICCCM section 2.1 */
    XSetSelectionOwner(dpy, sseln, m_win, CurrentTime);
    
    LOG4CXX_DEBUG(AppLogger, "ListenSelectionRequest running\n");
    m_threadStateMutex.lock();
    /*init thread start state*/
    m_threadState = THREAD_RUNNING;
    /* wait for a SelectionRequest event */
    while (m_threadState == THREAD_RUNNING) {
        static unsigned int clear = 0;
        static unsigned int context = XCLIB_XCIN_NONE;
        static unsigned long sel_pos = 0;
        static Window cwin;
        static Atom pty;
        int finished;
        
        XEvent evt = {0};
        
        XNextEvent(dpy, &evt);
        
        finished = xcin(dpy, &cwin, evt, &pty, target, sel_buf, sel_len, &sel_pos, &context);
        
        if (evt.type == SelectionClear)
            clear = 1;
        
        if ((context == XCLIB_XCIN_NONE) && clear)
            break;
        
        if (m_threadState == THREAD_STOPPING)
            break;
    }

    if(sel_buf)
        delete sel_buf;
    
    m_threadState = THREAD_STOPPED;
    m_threadStateMutex.unlock();
    LOG4CXX_DEBUG(AppLogger, "ListenSelectionRequest stopped\n");
    return EXIT_SUCCESS;
}
#endif
/* put data into a selection, in response to a SelecionRequest event from
 ** another window (and any subsequent events relating to an INCR transfer).
 **
 ** Arguments are:
 **
 ** A display
 **
 ** A window
 **
 ** The event to respond to
 **
 ** A pointer to an Atom. This gets set to the property nominated by the other
 ** app in it's SelectionRequest. Things are likely to break if you change the
 ** value of this yourself.
 **
 ** The target(UTF8_STRING or XA_STRING) to respond to
 **
 ** A pointer to an array of chars to read selection data from.
 **
 ** The length of the array of chars.
 **
 ** In the case of an INCR transfer, the position within the array of chars
 ** that is being processed.
 **
 ** The context that event is the be processed within.
 **/
#if 1
int
ClipBoardLinux::xcin(Display * dpy,
                     Window * win,
                     XEvent evt,
                     Atom * pty, unsigned long *pos,
                     unsigned int *context)
{
    unsigned long chunk_len;	/* length of current chunk (for incr
                                 * transfers only)
                                 * 				 */
    XEvent res;			/* response to event */
    static Atom inc;
    static Atom targets;
    static long chunk_size;
    
    static Atom target = None;
    //static unsigned char *txt = NULL;
    //static unsigned long len = 0;
    
    if (!targets) {
        targets = XInternAtom(dpy, "TARGETS", False);
    }
    
    if (!inc) {
        inc = XInternAtom(dpy, "INCR", False);
    }
    
    /* We consider selections larger than a quarter of the maximum
     *        request size to be "large". See ICCCM section 2.5 */
    if (!chunk_size) {
        chunk_size = XExtendedMaxRequestSize(dpy) / 4;
        if (!chunk_size) {
            chunk_size = XMaxRequestSize(dpy) / 4;
        }
    }
    
    if (evt.xselectionrequest.target == targets) {
        *context = XCLIB_XCIN_NONE;
    }
    else
    {
        unsigned int i, m_length;
        
        for (i = 0; i < m_clipboardItemVect.size(); ++i) {
            if (evt.xselectionrequest.target == m_clipboardItemVect[i].first) {
                target = evt.xselectionrequest.target;
#if 0
                if (target == XInternAtom(dpy, "text/rtf", False))
                {

                    m_selectionLen = m_length*2+1;
                    m_selectionBuf = (unsigned char*) malloc (m_selectionLen* sizeof(char));//new unsigned char[destLen];
                    memset(m_selectionBuf, 0x00, m_selectionLen);
                    int ret = uncompress(m_selectionBuf, &m_selectionLen, m_clipboardItemVect[i].second.data(), m_clipboardItemVect[i].second.size());
                    m_selectionLen ++;
                    if ( ret != Z_OK){
                        LOG4CXX_DEBUG(AppLogger, "uncompress error:"<< ret);
                        return 0;
                    }

                }
                else
#endif
                {
                    m_selectionLen = m_clipboardItemVect[i].second.size();
                    m_selectionBuf = (unsigned char*) malloc (m_selectionLen * sizeof(char));
                    memcpy(m_selectionBuf, m_clipboardItemVect[i].second.data(), m_selectionLen);
                }
                
                //m_selectionBuf = m_selectionBuf;
                break;
            }
        }

        if (m_selectionLen == 0) {
            return (0);
        }
    }
    
    if ((evt.type == SelectionRequest) || (evt.type == PropertyNotify)){
        char *atomName1 = NULL;
        char *atomName2 = NULL;
        if (dpy,evt.xselectionrequest.target != None){
            atomName1 = XGetAtomName(dpy,evt.xselectionrequest.target);
        }
        if (dpy,evt.xselectionrequest.property != None){
            atomName2 = XGetAtomName(dpy,evt.xselectionrequest.property);
        }
        if (atomName1 && atomName2){
            //if (atomName1){
            printf("m_selectionLen: %ld; chunk size: %ld; req-target: %s; req-property: %s; context: %d; \n", m_selectionLen, chunk_size, XGetAtomName(dpy,evt.xselectionrequest.target), XGetAtomName(dpy,evt.xselectionrequest.property), *context);
        }
    }
    
    switch (*context) {
        case XCLIB_XCIN_NONE:
            if (evt.type != SelectionRequest)
                return (0);
            
            /* set the window and property that is being used */
            *win = evt.xselectionrequest.requestor;
            *pty = evt.xselectionrequest.property;
            
            /* reset position to 0 */
            *pos = 0;
            
            /* put the data into an property */
            if (evt.xselectionrequest.target == targets) {
                std::vector<Atom> target_vec;
                target_vec.push_back(targets);
                target_vec.push_back(XInternAtom(dpy, "Kingsoft Data Descriptor", False));
                
                for (size_t i = 0; i < m_clipboardItemVect.size(); ++i) {
                    target_vec.push_back(m_clipboardItemVect[i].first);
                }
                
                /* send data all at once (not using INCR) */
                XChangeProperty(dpy,
                                *win,
                                *pty,
                                XA_ATOM,
                                32, PropModeReplace,
                                reinterpret_cast<unsigned char*>(&target_vec.front()),
                                target_vec.size()
                                );
            }
            else if (m_selectionLen > chunk_size) {
                /* send INCR response */
                XChangeProperty(dpy, *win, *pty, inc, 32, PropModeReplace, 0, 0);
                
                /* With the INCR mechanism, we need to know
                 * 	     * when the requestor window changes (deletes)
                 * 	     	     * its properties
                 * 	     	     	     */
                XSelectInput(dpy, *win, PropertyChangeMask);
                
                *context = XCLIB_XCIN_INCR;
            }
            else {
                /* send data all at once (not using INCR) */
                XChangeProperty(dpy,
                                *win,
                                *pty, target, 8, PropModeReplace, (unsigned char *) m_selectionBuf, (int) m_selectionLen);
            }
            
            /* Perhaps FIXME: According to ICCCM section 2.5, we should
             * 	   confirm that XChangeProperty succeeded without any Alloc
             * 	   	   errors before replying with SelectionNotify. However, doing
             * 	   	   	   so would require an error handler which modifies a global
             * 	   	   	   	   variable, plus doing XSync after each XChangeProperty. */
            
            /* set values for the response event */
            res.xselection.property = *pty;
            res.xselection.type = SelectionNotify;
            res.xselection.display = evt.xselectionrequest.display;
            res.xselection.requestor = *win;
            res.xselection.selection = evt.xselectionrequest.selection;
            res.xselection.target = evt.xselectionrequest.target;
            res.xselection.time = evt.xselectionrequest.time;
            
            /* send the response event */
            XSendEvent(dpy, evt.xselectionrequest.requestor, 0, 0, &res);
            XFlush(dpy);
            
            /* don't treat TARGETS request as contents request */
            if (evt.xselectionrequest.target == targets)
                return 0;
            
            /* if m_selectionLen < chunk_size, then the data was sent all at
             * 	 * once and the transfer is now complete, return 1
             * 	 	 */
            if (m_selectionLen > chunk_size)
                return (0);
            else
                return (1);
            
            break;
            
        case XCLIB_XCIN_INCR:
            /* length of current chunk */
            
            /* ignore non-property events */
            if (evt.type != PropertyNotify)
                return (0);
            
            /* ignore the event unless it's to report that the
             * 	 * property has been deleted
             * 	 	 */
            if (evt.xproperty.state != PropertyDelete)
                return (0);
            
            /* set the chunk length to the maximum size */
            chunk_len = chunk_size;
            
            /* if a chunk length of maximum size would extend
             * 	 * beyond the end ot txt, set the length to be the
             * 	 	 * remaining length of txt
             * 	 	 	 */
            if ((*pos + chunk_len) > m_selectionLen)
                chunk_len = m_selectionLen - *pos;
            
            /* if the start of the chunk is beyond the end of txt,
             * 	 * then we've already sent all the data, so set the
             * 	 	 * length to be zero
             * 	 	 	 */
            if (*pos > m_selectionLen)
                chunk_len = 0;
            
            if (chunk_len) {
                /* put the chunk into the property */
                XChangeProperty(dpy,
                                *win, *pty, target, 8, PropModeReplace, &m_selectionBuf[*pos], (int) chunk_len);
            }
            else {
                /* make an empty property to show we've
                 * 	     * finished the transfer
                 * 	     	     */
                XChangeProperty(dpy, *win, *pty, target, 8, PropModeReplace, 0, 0);
            }
            XFlush(dpy);
            
            /* all data has been sent, break out of the loop */
            if (!chunk_len)
                *context = XCLIB_XCIN_NONE;
            
            *pos += chunk_size;
            
            /* if chunk_len == 0, we just finished the transfer,
             * 	 * return 1
             * 	 	 */
            if (chunk_len > 0)
                return (0);
            else
                return (1);
            break;
    }
    return (0);
}

#else
int
ClipBoardLinux::xcin(Display * dpy,
                     Window * win,
                     XEvent evt,
                     Atom * pty, Atom target, unsigned char *txt, unsigned long len, unsigned long *pos,
                     unsigned int *context)
{
    unsigned long chunk_len;	/* length of current chunk (for incr
                                 * transfers only)
                                 * 				 */
    XEvent res;			/* response to event */
    static Atom inc;
    static Atom targets;
    static long chunk_size;
    
    if (!targets) {
        targets = XInternAtom(dpy, "TARGETS", False);
    }
    
    if (!inc) {
        inc = XInternAtom(dpy, "INCR", False);
    }
    
    /* We consider selections larger than a quarter of the maximum
     *        request size to be "large". See ICCCM section 2.5 */
    if (!chunk_size) {
        chunk_size = XExtendedMaxRequestSize(dpy) / 4;
        if (!chunk_size) {
            chunk_size = XMaxRequestSize(dpy) / 4;
        }
    }
    if ((evt.xselectionrequest.target == targets)) {
        *context = XCLIB_XCIN_NONE;
    }
    
    if ((evt.type == SelectionRequest) || (evt.type == PropertyNotify)){
        char *atomName1 = NULL;
        char *atomName2 = NULL;
        if (dpy,evt.xselectionrequest.target != None){
            atomName1 = XGetAtomName(dpy,evt.xselectionrequest.target);
        }
        if (dpy,evt.xselectionrequest.property != None){
            atomName2 = XGetAtomName(dpy,evt.xselectionrequest.property);
        }
        if (atomName1 && atomName2){
        //if (atomName1){
            printf("len: %ld; chunk size: %ld; req-target: %s; req-property: %s; context: %d; \n", len, chunk_size, XGetAtomName(dpy,evt.xselectionrequest.target), XGetAtomName(dpy,evt.xselectionrequest.property), *context);
        }
    }
    switch (*context) {
        case XCLIB_XCIN_NONE:
            if (evt.type != SelectionRequest)
                return (0);
            
            /* set the window and property that is being used */
            *win = evt.xselectionrequest.requestor;
            *pty = evt.xselectionrequest.property;
            
            /* reset position to 0 */
            *pos = 0;
            
            /* put the data into an property */
            if (evt.xselectionrequest.target == targets) {
                Atom types[17] = {
                    targets,
                    XInternAtom(dpy, "Kingsoft Data Descriptor", False),
                    XInternAtom(dpy, "Kingsoft WPS 9.0 Format", False),
                    XInternAtom(dpy, "Embed Source", False),
                    XInternAtom(dpy, "Object Descriptor", False),
                    XInternAtom(dpy, "Rich Text Format", False),
                    XInternAtom(dpy, "text/rtf", False),
                    XInternAtom(dpy, "text/richtext", False),
                    XInternAtom(dpy, "text/html", False),
                    XInternAtom(dpy, "text/plain", False),
                    XInternAtom(dpy, "STRING", False),
                    XInternAtom(dpy, "TEXT", False),
                    XInternAtom(dpy, "COMPOUND_TEXT", False),
                    XInternAtom(dpy, "TIMESTAMP", False),
                    XInternAtom(dpy, "SAVE_TARGETS", False),
                    XInternAtom(dpy, "MULTIPLE", False),
                    //target,
                    XA_UTF8_STRING(dpy) };
                
                /* send data all at once (not using INCR) */
                XChangeProperty(dpy,
                                *win,
                                *pty,
                                XA_ATOM,
                                32, PropModeReplace, (unsigned char *) types,
                                (int) (sizeof(types) / sizeof(Atom))
                                );
            }
            else if (len > chunk_size) {
                /* send INCR response */
                XChangeProperty(dpy, *win, *pty, inc, 32, PropModeReplace, 0, 0);
                
                /* With the INCR mechanism, we need to know
                 * 	     * when the requestor window changes (deletes)
                 * 	     	     * its properties
                 * 	     	     	     */
                XSelectInput(dpy, *win, PropertyChangeMask);
                
                *context = XCLIB_XCIN_INCR;
            }
            else {
                /* send data all at once (not using INCR) */
                XChangeProperty(dpy,
                                *win,
                                *pty, target, 8, PropModeReplace, (unsigned char *) txt, (int) len);
            }
            
            /* Perhaps FIXME: According to ICCCM section 2.5, we should
             * 	   confirm that XChangeProperty succeeded without any Alloc
             * 	   	   errors before replying with SelectionNotify. However, doing
             * 	   	   	   so would require an error handler which modifies a global
             * 	   	   	   	   variable, plus doing XSync after each XChangeProperty. */
            
            /* set values for the response event */
            res.xselection.property = *pty;
            res.xselection.type = SelectionNotify;
            res.xselection.display = evt.xselectionrequest.display;
            res.xselection.requestor = *win;
            res.xselection.selection = evt.xselectionrequest.selection;
            res.xselection.target = evt.xselectionrequest.target;
            res.xselection.time = evt.xselectionrequest.time;
            
            /* send the response event */
            XSendEvent(dpy, evt.xselectionrequest.requestor, 0, 0, &res);
            XFlush(dpy);
            
            /* don't treat TARGETS request as contents request */
            if (evt.xselectionrequest.target == targets)
                return 0;
            
            /* if len < chunk_size, then the data was sent all at
             * 	 * once and the transfer is now complete, return 1
             * 	 	 */
            if (len > chunk_size)
                return (0);
            else
                return (1);
            
            break;
            
        case XCLIB_XCIN_INCR:
            /* length of current chunk */
            
            /* ignore non-property events */
            if (evt.type != PropertyNotify)
                return (0);
            
            /* ignore the event unless it's to report that the
             * 	 * property has been deleted
             * 	 	 */
            if (evt.xproperty.state != PropertyDelete)
                return (0);
            
            /* set the chunk length to the maximum size */
            chunk_len = chunk_size;
            
            /* if a chunk length of maximum size would extend
             * 	 * beyond the end ot txt, set the length to be the
             * 	 	 * remaining length of txt
             * 	 	 	 */
            if ((*pos + chunk_len) > len)
                chunk_len = len - *pos;
            
            /* if the start of the chunk is beyond the end of txt,
             * 	 * then we've already sent all the data, so set the
             * 	 	 * length to be zero
             * 	 	 	 */
            if (*pos > len)
                chunk_len = 0;
            
            if (chunk_len) {
                /* put the chunk into the property */
                XChangeProperty(dpy,
                                *win, *pty, target, 8, PropModeReplace, &txt[*pos], (int) chunk_len);
            }
            else {
                /* make an empty property to show we've
                 * 	     * finished the transfer
                 * 	     	     */
                XChangeProperty(dpy, *win, *pty, target, 8, PropModeReplace, 0, 0);
            }
            XFlush(dpy);
            
            /* all data has been sent, break out of the loop */
            if (!chunk_len)
                *context = XCLIB_XCIN_NONE;
            
            *pos += chunk_size;
            
            /* if chunk_len == 0, we just finished the transfer,
             * 	 * return 1
             * 	 	 */
            if (chunk_len > 0)
                return (0);
            else
                return (1);
            break;
    }
    return (0);
}
#endif

void ClipBoardLinux::ReceiveClientClipboardData(const string& clipType, const string& clipData)
{
    printf("clipType: %s, clipData size:%ld\n", clipType.c_str(), clipData.length());
    
    unsigned int clipFormat;
    Atom target;
    Display *dpy;
    
    dpy = XOpenDisplay(NULL);
    
    LOG4CXX_DEBUG(AppLogger, "set m_threadState THREAD_STOPPING\n");
    //check thread state, stop last selection request listen thread
    if(m_threadState == THREAD_RUNNING)
    {
        m_threadState = THREAD_STOPPING;
        LOG4CXX_DEBUG(AppLogger, "send wakeup event");
        Display *dpy = XOpenDisplay (NULL);
        XClientMessageEvent dummyEvent;
        memset(&dummyEvent, 0, sizeof(XClientMessageEvent));
        dummyEvent.type = ClientMessage;
        dummyEvent.window = m_win;
        dummyEvent.format = 32;
        XSendEvent(dpy, m_win, 0, 0, (XEvent*)&dummyEvent);
        XFlush(dpy);
        XCloseDisplay (dpy);
    }
    
    if (clipType == "TEXT")
        target = XA_UTF8_STRING(dpy);//clipFormat = kClipFormatText;
#if 0
    else if (clipType == "METAFILEPICT")
        clipFormat = CF_METAFILEPICT;
    else if (clipType == "SYLK")
        clipFormat = CF_SYLK;
    else if (clipType == "DIF")
        clipFormat = CF_DIF;
    else if (clipType == "TIFF")
        clipFormat = CF_TIFF;
    else if (clipType == "OEMTEXT")
        clipFormat = CF_OEMTEXT;
    else if (clipType == "DIB")
        clipFormat = CF_DIB;
    else if (clipType == "PALETTE")
        clipFormat = CF_PALETTE;
    else if (clipType == "PENDATA")
        clipFormat = CF_PENDATA;
    else if (clipType == "RIFF")
        clipFormat = CF_RIFF;
    else if (clipType == "WAVE")
        clipFormat = CF_WAVE;
    else if (clipType == "UNICODETEXT")
        clipFormat = CF_UNICODETEXT;
    else if (clipType == "ENHMETAFILE")
        clipFormat = CF_ENHMETAFILE;
    else if (clipType == "HDROP")
        clipFormat = CF_HDROP;
    else if (clipType == "LOCALE")
        clipFormat = CF_LOCALE;
    else if (clipType == "DIBV5")
        clipFormat = CF_DIBV5;
    else if (clipType == "OWNERDISPLAY")
        clipFormat = CF_OWNERDISPLAY;
    else if (clipType == "DSPTEXT")
        clipFormat = CF_DSPTEXT;
    else if (clipType == "DSPBITMAP")
        clipFormat = CF_DSPBITMAP;
    else if (clipType == "DSPMETAFILEPICT")
        clipFormat = CF_DSPMETAFILEPICT;
    else if (clipType == "DSPENHMETAFILE")
        clipFormat = CF_DSPENHMETAFILE;
    else if (clipType == "PRIVATEFIRST")
        clipFormat = CF_PRIVATEFIRST;
    else if (clipType == "PRIVATELAST")
        clipFormat = CF_PRIVATELAST;
    else if (clipType == "GDIOBJFIRST")
        clipFormat = CF_GDIOBJFIRST;
    else if (clipType == "GDIOBJLAST")
        clipFormat = CF_GDIOBJLAST;
#endif
    else if (clipType == "HTML")
        target = XInternAtom(dpy, "text/html", False);//clipFormat = kClipFormatHtml;
    else if (clipType == "RTF")
        target = XInternAtom(dpy, "text/rtf", False);//clipFormat = kClipFormatRtf;
    else if (clipType == "PNG")
        target = XInternAtom(dpy, "image/png", False);//clipFormat = kClipFormatPng;
    else if (clipType == "BITMAP")
        target = XInternAtom(dpy, "image/bmp", False);//clipFormat = kClipFormatBmp;
    else
        target = XA_UTF8_STRING(dpy);//clipFormat = kClipFormatNone;
    
    m_threadStateMutex.lock();
    std::vector<unsigned char> postClipData;
    base64_decodebytes(clipData, postClipData);

#if 0
    if ((clipType == "RTF") && m_length)
    {
        unsigned long destLen = m_length*2+1;
        unsigned char* dest = new unsigned char[destLen];
        memset(dest, 0x00, destLen);
        int ret = uncompress(dest, &destLen, clipData.data(), clipData.size());
        if ( ret == Z_OK) {
            m_selectionLen = destLen + 1;
            m_selectionBuf = dest;
        }
        else
        {
            LOG4CXX_DEBUG(AppLogger, "uncompress error:"<< ret);
        }
    }


    //Debuging codes
    //if (clipFormat == kClipFormatRtf){
    if (0){
        char* pFilename = "ClipboardData";
        static int num = 0;
        num ++;
        char filefullname[256] = {0};
        sprintf(filefullname, "%s-%d", pFilename, num);
        
        FILE *pFile = fopen(filefullname, "wb");
        fwrite((void*)m_selectionBuf, 1, m_selectionLen, pFile);
        fclose(pFile);
        printf("Wrote %s\n", filefullname);
    }
#endif

    
    std::pair<Atom,std::vector<unsigned char>> clipboardItem;
    clipboardItem.first = target;
    clipboardItem.second = postClipData;
    m_clipboardItemVect.push_back(clipboardItem);
    m_threadStateMutex.unlock();
    return;
}

